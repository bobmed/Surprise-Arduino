#ifndef GyverTimer_h
#define GyverTimer_h
#include <Arduino.h>

/*
	GTimer - полноценный таймер на базе системных millis() / micros()
	Документация: https://alexgyver.ru/gyvertimer/
	- Миллисекундный и микросекундный таймер
	- Два режима работы:
		- Режим интервала: таймер "срабатывает" каждый заданный интервал времени
		- Режим таймаута: таймер "срабатывает" один раз по истечении времени (до следующего перезапуска)
	- Служебные функции:
		- Старт
		- Стоп
		- Сброс
		- Продолжить
	
	Версии:
	- 2.0 - улучшенный алгоритм работы таймера
		- Кратные интервалы
		- Защита от пропусков
		- Защита от переполнения millis()
		- Убраны дефайны
	- 2.1 - возвращены дефайны
	- 2.2 - улучшена стабильность
	
	- 3.0
		- Логика работы разделена на интервал и таймаут
		- Добавлен общий класс GTimer (для миллисекундного и микросекундного таймера)
		- Добавлена возможность остановить и продолжить счёт таймера
	- 3.2
		- Добавлен isEnabled
		- Возможность не запускать таймер при создании
*/

enum timerType {
	US,
	MS,
};

// ============== GTimer (микросекундный и миллисекундный таймер) ================
class GTimer {
  public:
	GTimer(timerType type = MS, uint32_t interval = 0);	// объявление таймера с указанием типа и интервала (таймер не запущен, если не указывать)
	void setInterval(uint32_t interval);	// установка интервала работы таймера (также запустит и сбросит таймер) - режим интервала
	void setTimeout(uint32_t timeout);		// установка таймаута работы таймера (также запустит и сбросит таймер) - режим таймаута
	boolean isReady();						// возвращает true, когда пришло время
	boolean isEnabled();					// вернуть состояние таймера (остановлен/запущен)
	void reset();							// сброс таймера на установленный период работы
	void start();							// запустить/перезапустить (со сбросом счёта)
	void stop();							// остановить таймер (без сброса счёта)	
	void resume();							// продолжить (без сброса счёта)	

  //void myFuncAftrDeepSlpStgs(uint32_t sp); //Set values after deep sleep
  //uint32_t myFuncGetSP();
  
	// служебное
	void setMode(boolean mode);				// установка режима работы вручную: AUTO или MANUAL (TIMER_INTERVAL / TIMER_TIMEOUT)
	
  private:
	uint32_t _timer = 0;
	uint32_t _interval = 0;
	uint32_t _resumeBuffer = 0;
	boolean _mode = true;
	boolean _state = false;
	boolean _type = true;
};

#define MANUAL 0
#define AUTO 1
#define TIMER_TIMEOUT 0
#define TIMER_INTERVAL 1


#endif
