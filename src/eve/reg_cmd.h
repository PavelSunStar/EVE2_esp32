#pragma once

/*
 * Команды сопроцессора BT817/BT818 (EVE4).
 *
 * Это 32-битные идентификаторы команд, записываемые в FIFO RAM_CMD.
 * После идентификатора записываются параметры конкретной команды.
 * Общий размер команды вместе с параметрами и данными выравнивается до 4 байт.
 *
 * Важно:
 *   - это не Host Commands (ACTIVE, CLKEXT и т. п.);
 *   - это не Display List команды (CLEAR, DISPLAY, VERTEX2F и т. п.);
 *   - BT817/BT818 после запуска использует API level 1;
 *   - для новых возможностей BT817/BT818 следует вызвать CMD_APILEVEL(2).
 */

// =============================================================================
// Управление Display List и синхронизация
// =============================================================================

#define CMD_DLSTART             0xFFFFFF00UL  // Начать формирование нового Display List.
#define CMD_SWAP                0xFFFFFF01UL  // Показать сформированный Display List на следующем кадре.
#define CMD_INTERRUPT           0xFFFFFF02UL  // Создать INT_CMDFLAG через заданное число миллисекунд.
#define CMD_APPEND              0xFFFFFF1EUL  // Добавить блок готовых DL-команд из RAM_G в текущий Display List.
#define CMD_SYNC                0xFFFFFF42UL  // Дождаться окончания текущего периода сканирования экрана.
#define CMD_APPENDF             0xFFFFFF59UL  // Добавить DL-команды непосредственно из внешней Flash.

// =============================================================================
// Цвета, градиенты и графические виджеты
// =============================================================================

#define CMD_BGCOLOR             0xFFFFFF09UL  // Установить цвет фона для 3D-виджетов.
#define CMD_FGCOLOR             0xFFFFFF0AUL  // Установить основной цвет 3D-виджетов.
#define CMD_GRADIENT            0xFFFFFF0BUL  // Нарисовать линейный RGB-градиент.
#define CMD_TEXT                0xFFFFFF0CUL  // Нарисовать строку UTF-8.
#define CMD_BUTTON              0xFFFFFF0DUL  // Нарисовать кнопку с подписью UTF-8.
#define CMD_KEYS                0xFFFFFF0EUL  // Нарисовать ряд клавиш.
#define CMD_PROGRESS            0xFFFFFF0FUL  // Нарисовать индикатор выполнения.
#define CMD_SLIDER              0xFFFFFF10UL  // Нарисовать ползунок.
#define CMD_SCROLLBAR           0xFFFFFF11UL  // Нарисовать полосу прокрутки.
#define CMD_TOGGLE              0xFFFFFF12UL  // Нарисовать переключатель с двумя подписями.
#define CMD_GAUGE               0xFFFFFF13UL  // Нарисовать стрелочный индикатор.
#define CMD_CLOCK               0xFFFFFF14UL  // Нарисовать аналоговые часы.
#define CMD_DIAL                0xFFFFFF2DUL  // Нарисовать вращающийся регулятор.
#define CMD_NUMBER              0xFFFFFF2EUL  // Нарисовать целое число.
#define CMD_GRADCOLOR           0xFFFFFF34UL  // Установить цвет блика для 3D-виджетов.
#define CMD_SETBASE             0xFFFFFF38UL  // Установить основание системы счисления для CMD_NUMBER.
#define CMD_GRADIENTA           0xFFFFFF57UL  // Нарисовать градиент с альфа-каналом.
#define CMD_FILLWIDTH           0xFFFFFF58UL  // Задать ширину заполнения/переноса текста.

// =============================================================================
// Матрица преобразования и настройка bitmap
// =============================================================================

#define CMD_BITMAP_TRANSFORM    0xFFFFFF21UL  // Рассчитать матрицу преобразования bitmap по трём точкам.
#define CMD_LOADIDENTITY        0xFFFFFF26UL  // Сбросить текущую bitmap-матрицу в единичную.
#define CMD_TRANSLATE           0xFFFFFF27UL  // Добавить смещение к текущей bitmap-матрице.
#define CMD_SCALE               0xFFFFFF28UL  // Добавить масштабирование к текущей bitmap-матрице.
#define CMD_ROTATE              0xFFFFFF29UL  // Добавить вращение к текущей bitmap-матрице.
#define CMD_SETMATRIX           0xFFFFFF2AUL  // Записать текущую матрицу в Display List.
#define CMD_GETMATRIX           0xFFFFFF33UL  // Получить шесть коэффициентов текущей bitmap-матрицы.
#define CMD_SETBITMAP           0xFFFFFF43UL  // Сформировать DL-настройки bitmap по адресу, формату и размеру.
#define CMD_ROTATEAROUND        0xFFFFFF51UL  // Повернуть и масштабировать bitmap вокруг заданной точки.

// =============================================================================
// Операции с регистрами и памятью RAM_G
// =============================================================================

#define CMD_MEMCRC              0xFFFFFF18UL  // Рассчитать CRC-32 указанного блока памяти.
#define CMD_REGREAD             0xFFFFFF19UL  // Прочитать 32-битное значение регистра через сопроцессор.
#define CMD_MEMWRITE            0xFFFFFF1AUL  // Записать следующие inline-данные по указанному адресу.
#define CMD_MEMSET              0xFFFFFF1BUL  // Заполнить блок памяти указанным 8-битным значением.
#define CMD_MEMZERO             0xFFFFFF1CUL  // Заполнить блок памяти нулями.
#define CMD_MEMCPY              0xFFFFFF1DUL  // Скопировать блок памяти внутри адресного пространства EVE.
#define CMD_GETPTR              0xFFFFFF23UL  // Получить первый свободный адрес после CMD_INFLATE.

// =============================================================================
// Загрузка, распаковка и получение параметров изображений
// =============================================================================

#define CMD_INFLATE             0xFFFFFF22UL  // Распаковать следующие inline-данные ZLIB/DEFLATE в RAM_G.
#define CMD_LOADIMAGE           0xFFFFFF24UL  // Декодировать JPEG/PNG в RAM_G.
#define CMD_GETPROPS            0xFFFFFF25UL  // Получить адрес, ширину и высоту последнего CMD_LOADIMAGE.
#define CMD_MEDIAFIFO           0xFFFFFF39UL  // Назначить область RAM_G потоковым Media FIFO.
#define CMD_INFLATE2            0xFFFFFF50UL  // Распаковать данные из FIFO, Media FIFO или внешней Flash.
#define CMD_GETIMAGE            0xFFFFFF64UL  // Получить полные параметры последнего декодированного изображения.

// =============================================================================
// Видео MJPEG
// =============================================================================

#define CMD_PLAYVIDEO           0xFFFFFF3AUL  // Воспроизвести MJPEG AVI до завершения.
#define CMD_VIDEOSTART          0xFFFFFF40UL  // Запустить покадровый декодер видео из Media FIFO.
#define CMD_VIDEOFRAME          0xFFFFFF41UL  // Декодировать следующий кадр видео в RAM_G.
#define CMD_VIDEOSTARTF         0xFFFFFF5FUL  // Запустить покадровый декодер видео из внешней Flash.

// =============================================================================
// Внешняя Flash-память EVE
// =============================================================================

#define CMD_FLASHERASE          0xFFFFFF44UL  // Полностью стереть внешнюю Flash.
#define CMD_FLASHWRITE          0xFFFFFF45UL  // Записать следующие inline-данные во Flash.
#define CMD_FLASHREAD           0xFFFFFF46UL  // Скопировать данные из Flash в RAM_G.
#define CMD_FLASHUPDATE         0xFFFFFF47UL  // Обновить блок Flash с автоматическим стиранием.
#define CMD_FLASHDETACH         0xFFFFFF48UL  // Отключить Flash от контроллера EVE.
#define CMD_FLASHATTACH         0xFFFFFF49UL  // Подключить Flash к контроллеру EVE.
#define CMD_FLASHFAST           0xFFFFFF4AUL  // Перевести Flash в быстрый режим и вернуть результат.
#define CMD_FLASHSPIDESEL       0xFFFFFF4BUL  // Снять выбор устройства на внутренней SPI-шине Flash.
#define CMD_FLASHSPITX          0xFFFFFF4CUL  // Передать произвольные байты через внутреннюю SPI-шину Flash.
#define CMD_FLASHSPIRX          0xFFFFFF4DUL  // Принять байты через внутреннюю SPI-шину Flash в RAM_G.
#define CMD_FLASHSOURCE         0xFFFFFF4EUL  // Задать адрес Flash для следующей команды чтения/декодирования.
#define CMD_CLEARCACHE          0xFFFFFF4FUL  // Очистить кэш внешней Flash.
#define CMD_FLASHPROGRAM        0xFFFFFF70UL  // Записать RAM_G во Flash без стирания; область должна быть чистой.

// =============================================================================
// Шрифты и внутренние bitmap-ресурсы виджетов
// =============================================================================

#define CMD_SETFONT             0xFFFFFF2BUL  // Зарегистрировать старый формат пользовательского шрифта.
#define CMD_ROMFONT             0xFFFFFF3FUL  // Назначить встроенный ROM-шрифт указанному bitmap handle.
#define CMD_SETFONT2            0xFFFFFF3BUL  // Зарегистрировать расширенный пользовательский шрифт.
#define CMD_SETSCRATCH          0xFFFFFF3CUL  // Выбрать bitmap handle для внутренних ресурсов виджетов.
#define CMD_RESETFONTS          0xFFFFFF52UL  // Восстановить стандартные ROM-шрифты в handles 16–31.
#define CMD_FONTCACHE           0xFFFFFF6BUL  // Создать в RAM_G кэш glyph-изображений Flash-шрифта.
#define CMD_FONTCACHEQUERY      0xFFFFFF6CUL  // Получить размер и использование кэша шрифта.

// =============================================================================
// Touch: калибровка и отслеживание объектов
// =============================================================================

#define CMD_CALIBRATE           0xFFFFFF15UL  // Выполнить полную интерактивную калибровку touch.
#define CMD_TRACK               0xFFFFFF2CUL  // Включить отслеживание касания для заданной области/виджета.
#define CMD_CALIBRATESUB        0xFFFFFF60UL  // Выполнить калибровку touch внутри заданного окна.

// =============================================================================
// Встроенные эффекты, снимки и служебные экраны
// =============================================================================

#define CMD_SPINNER             0xFFFFFF16UL  // Запустить анимированный индикатор ожидания.
#define CMD_STOP                0xFFFFFF17UL  // Остановить spinner, screensaver или sketch.
#define CMD_SNAPSHOT            0xFFFFFF1FUL  // Снять копию всего экрана в RAM_G.
#define CMD_SCREENSAVER         0xFFFFFF2FUL  // Запустить встроенную заставку.
#define CMD_SKETCH              0xFFFFFF30UL  // Запустить непрерывное рисование касаниями в bitmap.
#define CMD_LOGO                0xFFFFFF31UL  // Показать встроенную анимацию логотипа EVE.
#define CMD_SETROTATE           0xFFFFFF36UL  // Повернуть экран и соответствующим образом настроить touch.
#define CMD_SNAPSHOT2           0xFFFFFF37UL  // Снять область экрана с выбором формата результата.
#define CMD_TESTCARD            0xFFFFFF61UL  // Сформировать и показать встроенную тестовую таблицу.

// =============================================================================
// Анимации ASTC
// =============================================================================

#define CMD_ANIMSTART           0xFFFFFF53UL  // Запустить анимацию, размещённую во внешней Flash.
#define CMD_ANIMSTOP            0xFFFFFF54UL  // Остановить выбранный канал анимации.
#define CMD_ANIMXY              0xFFFFFF55UL  // Изменить координаты активной анимации.
#define CMD_ANIMDRAW            0xFFFFFF56UL  // Добавить все активные анимации в Display List.
#define CMD_ANIMFRAME           0xFFFFFF5AUL  // Нарисовать один кадр анимации из внешней Flash.
#define CMD_ANIMFRAMERAM        0xFFFFFF6DUL  // Нарисовать один кадр анимации из RAM_G.
#define CMD_ANIMSTARTRAM        0xFFFFFF6EUL  // Запустить анимацию, размещённую в RAM_G.
#define CMD_RUNANIM             0xFFFFFF6FUL  // Проиграть анимацию до полного завершения.

// =============================================================================
// Command List — списки команд, сохранённые в RAM_G (BT817/BT818)
// =============================================================================

#define CMD_RETURN              0xFFFFFF66UL  // Завершить выполняемый command list и вернуться к вызывающему.
#define CMD_CALLLIST            0xFFFFFF67UL  // Выполнить ранее скомпилированный command list из RAM_G.
#define CMD_NEWLIST             0xFFFFFF68UL  // Начать компиляцию command list в RAM_G.
#define CMD_ENDLIST             0xFFFFFF69UL  // Завершить компиляцию command list.

// =============================================================================
// Управление сопроцессором и расширения BT817/BT818
// =============================================================================

#define CMD_COLDSTART           0xFFFFFF32UL  // Сбросить состояние сопроцессора к значениям по умолчанию.
#define CMD_NOP                 0xFFFFFF5BUL  // Ничего не делать; безопасный заполнитель FIFO.
#define CMD_HSF                 0xFFFFFF62UL  // Задать ширину для режима Horizontal Scanout Filter.
#define CMD_APILEVEL            0xFFFFFF63UL  // Выбрать API level: 1 — BT815, 2 — BT817/BT818.
#define CMD_WAIT                0xFFFFFF65UL  // Подождать заданное число микросекунд, максимум 1 000 000.
#define CMD_PCLKFREQ            0xFFFFFF6AUL  // Подобрать и установить ближайшую частоту пиксельного такта.

