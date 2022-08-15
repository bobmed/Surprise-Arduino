#include <Arduino.h>
#include "Timer.h"

// конструктор
GTimer::GTimer(timerType type, uint32_t interval) {
	setInterval(interval);		// запуск в режиме ИНТЕРВАЛА
	_type = type;				// установка типа
}

// запуск в режиме интервала
void GTimer::setInterval(uint32_t interval) {
	if (interval != 0) {				// защита от ввода 0
		_interval = interval;			// установка
		_mode = TIMER_INTERVAL;			// режим "интервал"
		start();						// сброс и запуск	
	} else stop();						// остановка, если время == 0
}

// запуск в режиме таймаута
void GTimer::setTimeout(uint32_t timeout) {
	setInterval(timeout);		// задать интервал и запустить
	_mode = TIMER_TIMEOUT;		// режим "таймаут"
}

// остановить счёт
void GTimer::stop() {
	_state = false;
	_resumeBuffer = ( (_type) ? millis() : micros() ) - _timer;		// запомнили "время остановки"
}

// продолжить счёт
void GTimer::resume() {
	start();
	_timer -= _resumeBuffer;	// восстановили время остановки
}

// перезапустить счёт
void GTimer::start() {
	_state = true;
	reset();
}

// сброс периода
void GTimer::reset() {
	_timer = (_type) ? millis() : micros();
}

// состояние
boolean GTimer::isEnabled() {
	return _state;
}

// проверка таймера v2.0 (соблюдение интервалов, защита от пропуска и переполнения)
boolean GTimer::isReady() {	
	if (!_state) return false;							// если таймер остановлен
	uint32_t thisTime = (_type) ? millis() : micros();	// текущее время
	if (thisTime - _timer >= _interval) {				// проверка времени
		if (_mode) {						// если режим интервалов
			do {
				_timer += _interval;
				if (_timer < _interval) break;			// защита от переполнения uint32_t
			} while (_timer < thisTime - _interval);	// защита от пропуска шага			
		} else {							// если режим таймаута
			_state = false;					// остановить таймер
		}
		return true;
	} else {
		return false;
	}
}

// сменить режим вручную
void GTimer::setMode(boolean mode) {
	_mode = mode;
}



/*
void GTimer::myFuncAftrDeepSlpStgs(uint32_t sp){
  _mode = TIMER_INTERVAL;
  _state = true;
  _type = true;
  _interval = 1000;
  _timer = sp;
}

uint32_t GTimer::myFuncGetSP(){
  return _timer;
}*/
