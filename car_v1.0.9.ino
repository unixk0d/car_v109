#include <Wire.h>
#include "U8glib.h" //Подключаем библиотеку для работы с дисплеем
#include "digits32.h" //Добавление  шрифтов 
#include "rus6x13B.h" //Добавление  шрифтов 
#include "my10x14rus.h" //Добавление  шрифтов 
#include  "rus4x6.h" //Добавление  шрифтов 
#include <SPI.h>

#include <DS3231.h>// часы реального времени
DS3231  rtc(SDA, SCL);

const int js0PinX = 0; // Аналог - Ось X джойстика №0
const int js0PinY = 1; // Аналог - Ось Y джойстика №0
const int js0PinSw = 2;  // Цыфра - кнопка джойстика №0
const int btnBTCall = 3; // Цыфра - прием сброс звонка на блютуз модуле
const int btnBTVUp = 4; // Цыфра - увеличение громкости на блютуз модуле
const int btnBTVDn = 5;  // Цыфра - уменьшение громкости на блютуз модуле

U8GLIB_ST7920_128X64_4X u8g(13, 11, 10); // Определяем пины соединения с дисплеем

// переменные для операторов логики, т.к. операторы логики не принимают указатели
boolean statStart = false; // определение перезапуска(0 - система только что запущина, 1 - прошел периуд инициализации устройств)
boolean statBt = false; //состояние блютуз модуля
boolean statLan = false; //состояние LAN модуля
boolean statD = false; //двежение вперед в данный момент времени (ДРАЙВ)
boolean statN = true; //двежение отсуствует в данный момент времени  (!!!НИТРАЛЬНАЯ!!! поумолчанию вкуюченна)
boolean statR = false; //двежение назад в данный момент времени  (РЕВЕРС)
boolean js0StatSw = false; // состояние кнопки джойстика №0 в данный момент времени
int statSpeed = 0; //скорость в данный момент времени
int statRpm = 0; //обороты двигателя в данный момент времени
int js0TempX = 512; //инициализируеться значением оси X только при нажатии соответствующей ей кнопки
int js0TempY = 700; //инициализируеться значением оси Y только при нажатии соответствующей ей кнопки, также выполняет требывания входа в начальное меню после инициализации
int js0StatX = 512; //значение оси X в данный момент времени
int js0StatY = 512; //значение оси Y в данный момент времени
int menuTemp = 0; //позиция в меню  в данный момент времени
int allAudioVol = 0; //общая громкасть аудиосистемы
int timeDow = 0; //инициализируеться значением дня недели в данный момент времени
int timeHour = 0; //инициализируеться значением часа в данный момент времени
int timeMin = 0; //инициализируеться значением минуты в данный момент времени
int timeSec = 0; //инициализируеться значением секунды в данный момент времени
int statError = 4; //инициализируеться значением количества ошибок в данный момент времени
int preloader = 0; //хранит конечное время отображения прелодера
float rubL = 1.9; //принимаемая в расчетах стоимость литра топлива в белорусских рублях.
float rubKm = 0.47; //стоимость траты топлива отображаемая стоимостью одного километра в данный момент времени

// Указатели - начало
boolean *pStatStart = &statStart;
boolean *pStatBt = &statBt;
boolean *pStatLan = &statLan;
boolean *pStatD = &statD;
boolean *pStatN = &statN;
boolean *pStatR = &statR;
boolean *pJs0StatSw = &js0StatSw;
int *pStatSpeed = &statSpeed;
int *pStatRpm = &statRpm;
int *pJs0TempX = &js0TempX;
int *pJs0TempY = &js0TempY;
int *pJs0StatX = &js0StatX;
int *pJs0StatY = &js0StatY;
int *pMenuTemp = &menuTemp;
int *pAllAudioVol = &allAudioVol;
int *pTimeDow = &timeDow;
int *pTimeHour = &timeHour;
int *pTimeMin = &timeMin;
int *pTimeSec = &timeSec;
int *pStatError = &statError;
int *pPreloader = &preloader;
float *pRubL = &rubL;
float *pRubKm = &rubKm;
// Указатели - конец



void fTimeGet() // Обработка показаний часов реального времени начало
{
  Time RTC_T;
  RTC_T = rtc.getTime();
  *pTimeDow = RTC_T.dow;
  *pTimeHour = RTC_T.hour;
  *pTimeMin = RTC_T.min;
  *pTimeSec = RTC_T.sec;
} // Обработка показаний часов реального времени конец



void fJs0 () // Обработка показаний джойсика начало
{
  // по умолчанию x=512, y=512 (провода вверх)
  // право x=512, y=1024
  // лево x=512, y=0
  // вверх x=0, y=512
  // вниз x=1024, y=512

  *pJs0StatSw = digitalRead(js0PinSw); // Считываем цифровое значение кнопки
  *pJs0StatX = analogRead(js0PinX); // Считываем аналоговое значение оси Х
  *pJs0StatY = analogRead(js0PinY); // Считываем аналоговое значение оси Y

  if (digitalRead(js0PinSw) == 0)
  {
    *pJs0TempX = analogRead(js0PinX); // Считываем аналоговое значение оси Х
    *pJs0TempY = analogRead(js0PinY); // Считываем аналоговое значение оси Y
  }
} // Обработка показаний джойсика конец



void fBT(int vol) // Блютуз обработка - начало
{
  if ( vol == 1) // Поднять/Опустить трубку подав на 100мс высокий потенциал на пин (также пауза/плей)
  {

    digitalWrite(btnBTCall, HIGH);
    delay(100);
    digitalWrite(btnBTCall, LOW);
    delay(100);

  }

  if ( vol == 2) // Увеличить громкость подав на 100мс высокий потенциал на пин
  {
    digitalWrite(btnBTVUp, LOW);
    delay(50);
    digitalWrite(btnBTVUp, HIGH);
    delay(60);
    digitalWrite(btnBTVUp, LOW);
    delay(50);
  }

  if ( vol == 3) // Уменьшить громкость подав на 100мс высокий потенциал на пин
  {
    digitalWrite(btnBTVDn, LOW);
    delay(50);
    digitalWrite(btnBTVDn, HIGH);
    delay(60);
    digitalWrite(btnBTVDn, LOW);
    delay(50);
  }

} // Блютуз обработка - конец



void fPreloader () // Функция вывода прелодера на дисплей начало
{
  Serial.print("loading...");
  u8g.firstPage();
  do {
    u8g.setFont(my10x14rus);
    u8g.setPrintPos(42, 40);
    u8g.print("PORTE");
    // Установка задержки в 5 секунд начало
    if (preloader == 0 && statStart == false) // Дополнительно исключаем зпнос 55+5=0 вторым параметром
    {
      (*pPreloader) = (*pTimeSec) + 5;
    }
    (*pPreloader) < (*pTimeSec) ? (*pStatStart) = true : (*pStatStart) = false;
    // Установка задержки в 5 секунд конец
  }
  while (u8g.nextPage());
  fBT(3);
} // Функция вывода прелодера на дисплей конец



void fMenuStat() // Верхние меню начало
{
  // Вывод времени начало
  u8g.setFont(rus4x6);
  u8g.setPrintPos(2, 7) ;
  u8g.print(timeHour);
  u8g.print(":");
  u8g.print(timeMin);
  u8g.print(":");
  u8g.print(timeSec);
  // Вывод времени конец

  if (statError) // Статус ошибок начало
  {
    if (timeSec % 2)
    {
      u8g.setColorIndex(1);
      u8g.drawBox(65, 1, 21, 7);
      u8g.setColorIndex(0);
      u8g.setPrintPos(66, 7) ;
      u8g.print("Er:");
      u8g.print(*pStatError);
      u8g.setColorIndex(1);
    }
    else
    {
      u8g.setPrintPos(66, 7) ;
      u8g.print("Er:");
      u8g.print(*pStatError);
    }
  } // Статус ошибок конец

  if (allAudioVol == 0) // Уровень звука начало
  {
    u8g.setColorIndex(1);
    u8g.drawBox(88, 1, 13, 7);
    u8g.setColorIndex(0);
    u8g.setPrintPos(89, 7) ;
    u8g.print(" 0 ");
    u8g.setColorIndex(1);
  }
  else
  {
    u8g.setPrintPos(89, 7) ;
    u8g.print(allAudioVol);
  } // Уровень звука конец

  if (statLan == false) // Статус сети LAN начало
  {
    u8g.setColorIndex(1);
    u8g.drawBox(103, 1, 13, 7);
    u8g.setColorIndex(0);
    u8g.setPrintPos(104, 7) ;
    u8g.print("lan");
    u8g.setColorIndex(1);
  }
  else
  {
    u8g.setPrintPos(104, 7) ;
    u8g.print("lan");
  } // Статус сети LAN конец

  if (statBt == false) // Статус сети BT начало
  {
    u8g.setColorIndex(1);
    u8g.drawBox(118, 1, 9, 7);
    u8g.setColorIndex(0);
    u8g.setPrintPos(119, 7) ;
    u8g.print("bt");
    u8g.setColorIndex(1);
  }
  else
  {
    u8g.setPrintPos(119, 7) ;
    u8g.print("bt");
  } // Статус сети BT конец

  u8g.drawLine(1, 9, 126, 9); // Разделитель
}// Верхние меню конец



void fMenu() // Остновное меню начало
{
  *pMenuTemp = 0;
  //Вывод скорости начало
  u8g.setFont (digits32);
  u8g.setPrintPos(2, 46) ;
  fSpeed ();
  u8g.print(statSpeed);
  u8g.setFont(my10x14rus);
  u8g.setPrintPos(18, 62) ;
  u8g.print("км/ч");
  // Вывод скорости конец
  u8g.drawLine(61, 9, 61, 62); // разделитель
  u8g.setFont (digits32);
  u8g.setPrintPos(64, 46) ;
  fRubKm ();
  u8g.print(rubKm);
  u8g.setFont(my10x14rus);
  u8g.setPrintPos(85, 62) ;
  u8g.print("р/км");
} // Остновное меню конец



void fMenuR() // Правое меню начало
{
  *pMenuTemp = 2;
  u8g.setFont(rus4x6);
  u8g.setPrintPos(40, 22) ;
  u8g.print("Аудио система.");
  u8g.setFont (digits32);
  u8g.setPrintPos(2, 60) ;
  u8g.print(allAudioVol);
}



void fMenuRC1() // Увелечение громкости начало
{
  allAudioVol < 22 ? allAudioVol++ : allAudioVol = 22;
  u8g.setFont(rus4x6);
  u8g.setPrintPos(40, 22) ;
  u8g.setFont (digits32);
  u8g.setPrintPos(2, 60) ;
  u8g.print(allAudioVol);
} // Увелечение громкости конец



void fMenuRC2() // Уменьшение громкости начало
{
  allAudioVol > 0 ? allAudioVol-- : allAudioVol = 0;
  u8g.setFont(rus4x6);
  u8g.setPrintPos(40, 22) ;
  u8g.setFont (digits32);
  u8g.setPrintPos(2, 60) ;
  u8g.print(allAudioVol);
} // Уменьшение громкости конец
// Правое меню конец



void fMenuUp() // Верхние меню начало
{
  *pMenuTemp = 3;
  u8g.setFont(rus4x6);
  u8g.setPrintPos(44, 22) ;
  u8g.print("Настройки.");
} // Верхние меню конец



void fMenuDn() // Нижнее меню начало
{
  *pMenuTemp = 4;
  u8g.setFont(rus4x6);
  u8g.setPrintPos(47, 22) ;
  u8g.print("Парковка.");
} // Нижнее меню конец



void fScreen () // Функция вывода на дисплей основного меню начало
{
  u8g.firstPage();
  do {
    fMenuStat();
    fBT(0);
    if (js0TempY > 650) fMenu();  // Остновное меню корень

    if ((menuTemp == 0 || menuTemp == 2) && js0TempY < 450)  fMenuR(); // Правое меню корень
    if (menuTemp == 2 && js0StatX < 450) // Правое меню - Увелечение громкости
    {
      fMenuRC1();
      fBT(2);
    }
    if (menuTemp == 2 && js0StatX > 650) // Правое меню - Уменьшение громкости
    {
      fMenuRC2();
      fBT(3);
    }

    if ((menuTemp == 0 || menuTemp == 3) && js0TempX < 450) fMenuUp(); // Верхние меню корень

    if ((menuTemp == 0 || menuTemp == 4) && js0TempX > 650) fMenuDn(); // Нижнее меню корень

    if ((js0TempX > 450 && js0TempX < 650) && (js0TempY > 450 && js0TempY < 650) && menuTemp == 0 && js0StatSw == false) fBT(1); // Поднять/Опустить трубку, Плей/Пауза из центрального меню

    //    Serial.print("menuTemp: ");
    //    Serial.print(menuTemp);
    //    Serial.print("\n");
    //    Serial.print("statBtnBTCall: ");
    //    Serial.print(statBtnBTCall);
    //    Serial.print("\n");
    //    Serial.print("js0StatSw: ");
    //    Serial.print(js0StatSw);
    //    Serial.print("\n");
  }
  while (u8g.nextPage());

} // Функция вывода на дисплей основного меню конец



void fSpeed () // Скорость начало
{
  statSpeed == 250 ? (*pStatSpeed = 0) : (*pStatSpeed)++; //демонстрационная функция догоняет значение до 250 и сбрасывает
} // Скорость - конец



void fRubKm () //  Стоимость траты топлива отображаемая стоимостью одного километра в данный момент времени - начало
{
  ;
} // Стоимость траты топлива отображаемая стоимостью одного километра в данный момент времени -  конец



void fRpm () // обороты двигателя - начало
{
  ;
} // Обороты двигателя - конец



void setup()
{
  pinMode(js0PinSw, INPUT); // Цыфра - кнопка джойстика №0
  digitalWrite(js0PinSw, HIGH); // Включаем встроенный подтягивающий резистор значение на пине всегда "1"
  pinMode(btnBTCall, OUTPUT); // Цыфра - прием сброс звонка на блютуз модуле
  pinMode(btnBTVUp, OUTPUT); // Цыфра - увеличение громкости на блютуз модуле
  pinMode(btnBTVDn, OUTPUT);  // Цыфра - уменьшение громкости на блютуз модуле

  rtc.begin(); // стартуем часы реального времени
  Serial.begin(9600); // Стартуем порт

  // Базовые установки LCD - начало
  u8g.setRot180();

  u8g.setHardwareBackup(u8g_backup_avr_spi);

  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }
  // Базовые установки LCD -  конец
}

void loop()
{
  fTimeGet(); // опрос часов реального времени
  fJs0 (); // опрос джойстика №0
  statStart == true ? fScreen () : fPreloader (); // запускаем меню если была инициализация
}
