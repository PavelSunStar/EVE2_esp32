import controlP5.*;
import java.io.*;
import java.util.*;
import java.util.zip.Deflater;

/*
 * EVE Converter for Processing 4
 *
 * Converts PNG/JPG/BMP images to an Arduino .h file containing:
 *   CMD_INFLATE + RAM_G address + zlib-compressed EVE pixel data.
 *
 * Modes:
 *   CELLS    - all images must have the same dimensions; one bitmap handle,
 *              images are selected with Vertex2ii(..., handle, cell).
 *   SEPARATE - images may have different dimensions; each image gets its own
 *              RAM_G address and (up to 32 images) its own handle.
 */

ControlP5 cp5;
ScrollableList formatList;
ScrollableList modeList;
Textfield nameField;
Toggle allFilesToggle;
Button openButton;

final int FORMAT_ARGB1555 = 0;
final int FORMAT_RGB565   = 1;
final int FORMAT_RGB332   = 2;
final int FORMAT_L8       = 3;

final int MODE_CELLS    = 0;
final int MODE_SEPARATE = 1;

final int CMD_INFLATE = 0xFFFFFF22;
final int RAM_G_LIMIT  = 1024 * 1024; // BT815/BT817: 1 MiB

int selectedFormat = FORMAT_ARGB1555;
int selectedMode = MODE_CELLS;

String statusText = "Выберите изображение";
PImage previewImage;

void setup() {
  size(650, 420);
  pixelDensity(1);
  surface.setTitle("EVE Asset Converter");
  surface.setResizable(true);

  cp5 = new ControlP5(this);

  openButton = cp5.addButton("openImages")
    .setPosition(12, 12)
    .setSize(130, 28)
    .setLabel("Открыть файл");

  formatList = cp5.addScrollableList("eveFormat")
    .setPosition(155, 12)
    .setSize(115, 130)
    .setBarHeight(28)
    .setItemHeight(24)
    .addItems(Arrays.asList("ARGB1555", "RGB565", "RGB332", "L8"))
    .setValue(0);

  modeList = cp5.addScrollableList("assetMode")
    .setPosition(282, 12)
    .setSize(155, 100)
    .setBarHeight(28)
    .setItemHeight(24)
    .addItems(Arrays.asList("Cells одинаковые", "Разные размеры"))
    .setValue(0);

  allFilesToggle = cp5.addToggle("allFiles")
    .setPosition(450, 12)
    .setSize(24, 24)
    .setLabel("Вся папка")
    .setValue(false);

  nameField = cp5.addTextfield("outputName")
    .setPosition(535, 12)
    .setSize(100, 28)
    .setLabel("Имя");

  textFont(createFont("Arial", 14));
}

void draw() {
  background(238);

  fill(20);
  textAlign(LEFT, TOP);
  textSize(14);
  text(statusText, 12, 60);

  if (previewImage != null) {
    float availableW = width - 24;
    float availableH = height - 105;
    float scale = min(availableW / previewImage.width,
                      availableH / previewImage.height);
    scale = min(scale, 8.0f);

    float drawW = previewImage.width * scale;
    float drawH = previewImage.height * scale;
    float px = (width - drawW) * 0.5f;
    float py = 92 + (availableH - drawH) * 0.5f;

    noSmooth();
    image(previewImage, px, py, drawW, drawH);
  }
}

void eveFormat(int value) {
  selectedFormat = value;
  formatList.close();
}

void assetMode(int value) {
  selectedMode = value;
  modeList.close();
}

void openImages() {
  selectInput("Выберите изображение", "fileSelected");
}

void fileSelected(File selectedFile) {
  if (selectedFile == null) {
    statusText = "Выбор отменён";
    return;
  }

  try {
    ArrayList<File> inputFiles = collectFiles(selectedFile);

    if (inputFiles.size() == 0) {
      throw new RuntimeException("Подходящие изображения не найдены");
    }

    String requestedName = trim(nameField.getText());
    if (requestedName.length() == 0) {
      requestedName = removeExtension(selectedFile.getName());
    }

    String symbolName = sanitizeIdentifier(requestedName);
    String macroName = symbolName.toUpperCase(Locale.ROOT);

    ArrayList<ImageAsset> assets = loadAssets(inputFiles);

    if (selectedMode == MODE_CELLS) {
      validateCellImages(assets);
    } else if (assets.size() > 32) {
      throw new RuntimeException(
        "Для режима разных размеров разрешено максимум 32 изображения (handle 0...31)"
      );
    }

    Conversion conversion = buildRawPixels(assets);

    if (conversion.raw.length > RAM_G_LIMIT) {
      throw new RuntimeException(
        "Данные занимают " + conversion.raw.length +
        " байт, а RAM_G имеет только " + RAM_G_LIMIT + " байт"
      );
    }

    byte[] compressed = compressZlib(conversion.raw);
    byte[] commandStream = makeInflateStream(compressed, 0);

    File outputFile = new File(
      selectedFile.getParentFile(),
      symbolName + "_assets.h"
    );

    writeHeader(
      outputFile,
      symbolName,
      macroName,
      assets,
      conversion,
      commandStream
    );

    previewImage = assets.get(0).image;

    statusText =
      "Готово: " + outputFile.getName() + "\n" +
      "Изображений: " + assets.size() +
      ", RAW: " + conversion.raw.length +
      " байт, zlib-пакет: " + commandStream.length + " байт";

    println(outputFile.getAbsolutePath());
  }
  catch (Exception error) {
    statusText = "Ошибка: " + error.getMessage();
    error.printStackTrace();
  }
}

ArrayList<File> collectFiles(File selectedFile) {
  ArrayList<File> result = new ArrayList<File>();

  if (!allFilesToggle.getState()) {
    if (isImageFile(selectedFile)) result.add(selectedFile);
    return result;
  }

  File[] folderFiles = selectedFile.getParentFile().listFiles();
  if (folderFiles == null) return result;

  Arrays.sort(folderFiles, new Comparator<File>() {
    public int compare(File a, File b) {
      return a.getName().compareToIgnoreCase(b.getName());
    }
  });

  for (File file : folderFiles) {
    if (file.isFile() && isImageFile(file)) result.add(file);
  }

  return result;
}

boolean isImageFile(File file) {
  String name = file.getName().toLowerCase(Locale.ROOT);
  return name.endsWith(".png")  ||
         name.endsWith(".jpg")  ||
         name.endsWith(".jpeg") ||
         name.endsWith(".bmp")  ||
         name.endsWith(".gif")  ||
         name.endsWith(".tif")  ||
         name.endsWith(".tiff");
}

ArrayList<ImageAsset> loadAssets(ArrayList<File> files) {
  ArrayList<ImageAsset> result = new ArrayList<ImageAsset>();

  for (File file : files) {
    PImage image = loadImage(file.getAbsolutePath());
    if (image == null) {
      throw new RuntimeException("Не удалось прочитать " + file.getName());
    }

    image.loadPixels();

    ImageAsset asset = new ImageAsset();
    asset.file = file;
    asset.image = image;
    asset.name = sanitizeIdentifier(removeExtension(file.getName()));
    asset.width = image.width;
    asset.height = image.height;
    result.add(asset);
  }

  return result;
}

void validateCellImages(ArrayList<ImageAsset> assets) {
  int width = assets.get(0).width;
  int height = assets.get(0).height;

  if (assets.size() > 128) {
    throw new RuntimeException("VERTEX2II поддерживает cell 0...127");
  }

  for (ImageAsset asset : assets) {
    if (asset.width != width || asset.height != height) {
      throw new RuntimeException(
        "Режим Cells требует одинаковый размер. " +
        asset.file.getName() + " имеет размер " +
        asset.width + "x" + asset.height +
        ", ожидалось " + width + "x" + height
      );
    }
  }
}

Conversion buildRawPixels(ArrayList<ImageAsset> assets) throws IOException {
  Conversion result = new Conversion();
  ByteArrayOutputStream raw = new ByteArrayOutputStream();

  for (int index = 0; index < assets.size(); index++) {
    ImageAsset asset = assets.get(index);

    // Separate images receive aligned RAM_G addresses. In CELLS mode there
    // must be no padding between frames because EVE computes the next cell as
    // stride * height.
    if (selectedMode == MODE_SEPARATE) {
      while ((raw.size() & 3) != 0) raw.write(0);
    }

    asset.address = raw.size();
    asset.handle = selectedMode == MODE_CELLS ? 0 : index;
    asset.cell = selectedMode == MODE_CELLS ? index : 0;
    asset.stride = asset.width * bytesPerPixel(selectedFormat);

    for (int pixel : asset.image.pixels) {
      writePixel(raw, pixel, selectedFormat);
    }
  }

  result.raw = raw.toByteArray();
  return result;
}

void writePixel(ByteArrayOutputStream output, int pixel, int format) {
  int a = (pixel >>> 24) & 0xFF;
  int r = (pixel >>> 16) & 0xFF;
  int g = (pixel >>> 8) & 0xFF;
  int b = pixel & 0xFF;

  if (format == FORMAT_ARGB1555) {
    int value =
      ((a >= 128 ? 1 : 0) << 15) |
      ((r >> 3) << 10) |
      ((g >> 3) << 5) |
      (b >> 3);

    output.write(value & 0xFF);
    output.write((value >>> 8) & 0xFF);
  }
  else if (format == FORMAT_RGB565) {
    int value =
      ((r >> 3) << 11) |
      ((g >> 2) << 5) |
      (b >> 3);

    output.write(value & 0xFF);
    output.write((value >>> 8) & 0xFF);
  }
  else if (format == FORMAT_RGB332) {
    int value =
      ((r >> 5) << 5) |
      ((g >> 5) << 2) |
      (b >> 6);

    output.write(value);
  }
  else { // L8
    int luminance = (r * 77 + g * 150 + b * 29) >> 8;
    output.write(luminance);
  }
}

byte[] compressZlib(byte[] source) throws IOException {
  Deflater deflater = new Deflater(Deflater.BEST_COMPRESSION);
  deflater.setInput(source);
  deflater.finish();

  ByteArrayOutputStream output = new ByteArrayOutputStream();
  byte[] buffer = new byte[4096];

  while (!deflater.finished()) {
    int count = deflater.deflate(buffer);
    output.write(buffer, 0, count);
  }

  deflater.end();
  return output.toByteArray();
}

byte[] makeInflateStream(byte[] compressed, int ramAddress) throws IOException {
  ByteArrayOutputStream output = new ByteArrayOutputStream();

  write32LE(output, CMD_INFLATE);
  write32LE(output, ramAddress);
  output.write(compressed);

  // EVE command streams must end on a 4-byte boundary.
  while ((output.size() & 3) != 0) output.write(0);

  return output.toByteArray();
}

void write32LE(ByteArrayOutputStream output, int value) {
  output.write(value & 0xFF);
  output.write((value >>> 8) & 0xFF);
  output.write((value >>> 16) & 0xFF);
  output.write((value >>> 24) & 0xFF);
}

void writeHeader(
  File outputFile,
  String symbolName,
  String macroName,
  ArrayList<ImageAsset> assets,
  Conversion conversion,
  byte[] stream
) throws IOException {
  PrintWriter writer = createWriter(outputFile.getAbsolutePath());

  writer.println("#pragma once");
  writer.println();
  writer.println("#include <Arduino.h>");
  writer.println();
  writer.println("// Generated by EVE_Converter.pde");
  writer.println("// Pixel format: " + formatName(selectedFormat));
  writer.println("// Uncompressed RAM_G bytes: " + conversion.raw.length);
  writer.println("// Images: " + assets.size());
  writer.println();

  writer.println("static const PROGMEM uint8_t " + symbolName + "_assets[" + stream.length + "] = {");

  writer.println("    // CMD_INFLATE = 0xFFFFFF22");
  writeHexRange(writer, stream, 0, 4);
  writer.println();

  writer.println("    // Destination address: RAM_G + 0");
  writeHexRange(writer, stream, 4, 8);
  writer.println();

  writer.println("    // ZLIB compressed pixel data (starts with 0x78)");
  writeHexRange(writer, stream, 8, stream.length);

  writer.println("};");
  writer.println();
  writer.println("#define " + macroName + "_FORMAT " + eveFormatConstant(selectedFormat));
  writer.println("#define " + macroName + "_IMAGE_COUNT " + assets.size());
  writer.println("#define " + macroName + "_RAM_G_SIZE " + conversion.raw.length + "UL");
  writer.println("#define LOAD_" + macroName + "() GD.copy(" + symbolName + "_assets, sizeof(" + symbolName + "_assets))");
  writer.println();

  writer.println("#ifndef EVE_IMAGE_ASSET_INFO_DEFINED");
  writer.println("#define EVE_IMAGE_ASSET_INFO_DEFINED");
  writer.println("struct EVEImageAssetInfo {");
  writer.println("    uint32_t address;");
  writer.println("    uint16_t width;");
  writer.println("    uint16_t height;");
  writer.println("    uint16_t stride;");
  writer.println("    uint8_t format;");
  writer.println("    uint8_t handle;");
  writer.println("    uint8_t cell;");
  writer.println("};");
  writer.println("#endif");
  writer.println();

  writer.println("static const EVEImageAssetInfo " + symbolName + "_images[" + assets.size() + "] = {");

  for (ImageAsset asset : assets) {
    writer.println(
      "    {" + asset.address + "UL, " +
      asset.width + ", " + asset.height + ", " + asset.stride + ", " +
      eveFormatConstant(selectedFormat) + ", " + asset.handle + ", " + asset.cell +
      "}, // " + asset.file.getName()
    );
  }

  writer.println("};");
  writer.println();

  if (selectedMode == MODE_CELLS) {
    ImageAsset first = assets.get(0);
    writer.println("#define " + macroName + "_HANDLE 0");
    writer.println("#define " + macroName + "_WIDTH " + first.width);
    writer.println("#define " + macroName + "_HEIGHT " + first.height);
    writer.println("#define " + macroName + "_STRIDE " + first.stride);
    writer.println("#define " + macroName + "_CELLS " + assets.size());
  }

  writer.flush();
  writer.close();
}

void writeHexRange(PrintWriter writer, byte[] bytes, int start, int end) {
  int column = 0;

  for (int i = start; i < end; i++) {
    if (column == 0) writer.print("    ");

    writer.print("0x" + hex(bytes[i] & 0xFF, 2));
    if (i + 1 < bytes.length) writer.print(", ");

    column++;
    if (column == 16 || i + 1 == end) {
      writer.println();
      column = 0;
    }
  }
}

int bytesPerPixel(int format) {
  return (format == FORMAT_ARGB1555 || format == FORMAT_RGB565) ? 2 : 1;
}

String formatName(int format) {
  if (format == FORMAT_ARGB1555) return "ARGB1555";
  if (format == FORMAT_RGB565) return "RGB565";
  if (format == FORMAT_RGB332) return "RGB332";
  return "L8";
}

String eveFormatConstant(int format) {
  return formatName(format);
}

String removeExtension(String name) {
  int dot = name.lastIndexOf('.');
  return dot > 0 ? name.substring(0, dot) : name;
}

String sanitizeIdentifier(String value) {
  String cleaned = value.replaceAll("[^A-Za-z0-9_]", "_");
  if (cleaned.length() == 0) cleaned = "eve_image";
  if (Character.isDigit(cleaned.charAt(0))) cleaned = "img_" + cleaned;
  return cleaned;
}

class ImageAsset {
  File file;
  PImage image;
  String name;
  int width;
  int height;
  int stride;
  int address;
  int handle;
  int cell;
}

class Conversion {
  byte[] raw;
}
