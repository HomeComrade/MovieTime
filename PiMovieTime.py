#!/usr/bin/env python

import time
from datetime import datetime, timedelta
import RPi.GPIO as GPIO
from neopixel import *

#Time configuration
UTC_DIFF = 2

# LED strip configuration:
LED_COUNT	 = 3	# Number of LED pixels.
LED_PIN		 = 18 # GPIO pin connected to the pixels (must support PWM!).
LED_FREQ_HZ = 800000 # LED signal frequency in hertz (usually 800khz)
LED_DMA		 = 5 # DMA channel to use for generating signal (try 5)
LED_INVERT	= False # True to invert the signal (when using NPN transistor level shift)

# Create NeoPixel object with appropriate configuration.
strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT)
# Intialize the library (must be called once before other functions).
strip.begin()

#Setup the light sensor
GPIO.setmode(GPIO.BCM)
PHOTORESISTOR_PIN = 23
LIGHT_READING_MEDIUM = 3500
LIGHT_READING_DARK = 20000

def getColor(maxBrightness):
	#for testing
	#UTC_DIFF = 0
	
	today = datetime.today()
	now = datetime.now() + timedelta(hours=UTC_DIFF)
	
	#for testing
	#now = now.replace(hour=21, minute=45, second=0, microsecond=0)
	
	#hold our important times
	weekday = today.weekday()
	hour = now.hour
	minute = now.minute
	
	#return values
	red = 0;
	green = 0;
	
	#defaults
	endGreen = now
	startRed = now
	
	#Sun, Mon, Tues, Wend, Thurs
	if weekday < 4 or weekday == 6:
		#between 09 and 21, movies are wonderful!
		if hour > 9 and hour < 21:
			return [0, maxBrightness, 0]
		#after 22:30, we have to go full red
		elif (hour == 22 and minute > 29) or hour > 22 or hour < 10:
			return [maxBrightness, 0, 0]
		
		endGreen = now.replace(hour=21, minute=0, second=0, microsecond=0)
		startRed = now.replace(hour=22, minute=30, second=0, microsecond=0)
	else:
		#from 09 till midnight, movies are wonderful!
		if hour > 8:
			return [0, maxBrightness, 0]
		#after 01:30 till 09, we have to go full red
		elif (hour == 1 and minute > 29) or (hour > 1 and hour < 9):
			return [maxBrightness, 0, 0]
		
		endGreen = now.replace(hour=00, minute=0, second=0, microsecond=0)
		startRed = now.replace(hour=1, minute=30, second=0, microsecond=0)
	
	stepIncrement = ((startRed - endGreen).seconds / maxBrightness)
	
	brightness = 0
	timeDiff = (now - endGreen).seconds
	
	while True:
		if (brightness * stepIncrement) < timeDiff:
			brightness += 1
		else:
			break
	
	#green end
	if brightness <= (maxBrightness / 2):
		green = maxBrightness
		red = brightness * 2
	else:
		green = (maxBrightness - brightness) * 2
		red = maxBrightness
	
	#normalize the colors. there is a bug but the day & time it happens is unclear at the moment
	if green > maxBrightness:
		green = maxBrightness
	elif green < 0:
		green = 0
	
	if red > maxBrightness:
		red = maxBrightness
	elif red < 0:
		red = 0
	
	#print 'now: '+str(now)+' endGreen: '+str(endGreen)+' startRed: '+str(startRed);
	#print 'stepIncrement: '+str(stepIncrement)+' brightness: '+str(brightness)+' timeDiff: '+str(timeDiff);
	#print 'hour: '+str(hour)+' minute: '+str(minute)+' red: '+str(red)+' green: '+str(green)
	
	return [red, green, 0]

# Main program logic follows:
if __name__ == '__main__':
	try:
		while True:
			#get the light reading
			lightReading = 0
			GPIO.setup(PHOTORESISTOR_PIN, GPIO.OUT)
			GPIO.output(PHOTORESISTOR_PIN, GPIO.LOW)
			time.sleep(0.1)
			
			GPIO.setup(PHOTORESISTOR_PIN, GPIO.IN)
			
			# This takes about 1 millisecond per loop cycle
			while (GPIO.input(PHOTORESISTOR_PIN) == GPIO.LOW):
				lightReading += 1
			
			maxBrightness = 255
			
			if lightReading > LIGHT_READING_MEDIUM and lightReading < LIGHT_READING_DARK:
				maxBrightness = 150
			elif lightReading > LIGHT_READING_DARK:
				maxBrightness = 100
			
			#print 'Reading: '+str(lightReading)+' maxBrightness: '+str(maxBrightness)
			
			color = getColor(maxBrightness);
			
			for i in range(0, LED_COUNT):
				strip.setPixelColor(i, Color(color[0], color[1], 0))
			
			strip.show()
			
			time.sleep(1)
	except KeyboardInterrupt:
		strip.setPixelColor(0, Color(0, 0, 0))
		strip.setPixelColor(1, Color(0, 0, 0))
		strip.setPixelColor(2, Color(0, 0, 0))
		strip.show()
