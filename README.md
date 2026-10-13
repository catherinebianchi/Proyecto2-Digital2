# Proyecto2-Digital2
Protocolos de Comunicación Serial y Timers

Es un sistema distribuido entre una computadora, una NUCLEO-F446RE y un ESP32. La Núcleo muestra un menú por UART y coordina al ESP32 mediante SPI e I2C. Además, genera una señal cuadrada con un timer. 

* UART: Menú con terminal serial y validación de comandos
* SPI: Opción 1 (comando LED,ms para encender uno de tres LEDs)
* I2C: Opción 2 (Núcleo pide la lectura del potenciómetro y lo muestra en la terminal)
* LCD: El ESP32 muestra voltaje, valor del ADC (0-255) y la inicial del último LED
* Timer: TIM6 genera una señal cuadrada sin retardos bloqueantes

Todas las partes funcionan simultáneamente: la señal cuadrada corre por interrupción y el ESP32 apaga sus LEDs con millis(). Igualmente, tras enviar un comando SPI, la Núcleo vuelve de inmediato al menú, para no bloquear la señal ni los nuevos comandos. 
