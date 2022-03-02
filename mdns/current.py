from Phidget22.PhidgetException import *
from Phidget22.Phidget import *
from Phidget22.Net import *
from Phidget22.Devices.Log import *
from Phidget22.LogLevel import *
from Phidget22.Devices.VoltageInput import *
import traceback
import time
import random

from datetime import datetime

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# You can generate an API token from the "API Tokens Tab" in the UI
token = "oVCPm7IyJn79ILFwHr7DPKrkEvVbc3zMVcaqIGXLl6-RnneIHwe9i2KbX_8MTQ5Q5rmGT-l-auXE80Km4QH1tg=="
org = "Baruch"
bucket = "sensors"
#Declare any event handlers here. These will be called every time the associated event occurs.

def onVoltageInput0_SensorChange(self, sensorValue, sensorUnit):
	print("SensorValue [0]: " + str(sensorValue))
	print("SensorUnit [0]: " + str(sensorUnit.symbol))
	print("----------")
	client=InfluxDBClient(url="http://localhost:8186", token=token, org=org)
	write_api = client.write_api(write_options=SYNCHRONOUS)
	point = Point("current") \
		.tag("sensor", "1") \
		.field("value", sensorValue) \
		.time(datetime.utcnow(), WritePrecision.NS)

	write_api.write(bucket, org, point)
	client.close()
	time.sleep(1)

def onVoltageInput0_Attach(self):
	print("Attach [0]!")

def onVoltageInput0_Detach(self):
	print("Detach [0]!")

def onVoltageInput0_Error(self, code, description):
	print("Code [0]: " + ErrorEventCode.getName(code))
	print("Description [0]: " + str(description))
	print("----------")

def onVoltageInput1_SensorChange(self, sensorValue, sensorUnit):
	print("SensorValue [1]: " + str(sensorValue))
	print("SensorUnit [1]: " + str(sensorUnit.symbol))
	print("----------")
	client=InfluxDBClient(url="http://localhost:8186", token=token, org=org)
	write_api = client.write_api(write_options=SYNCHRONOUS)
	point = Point("current") \
		.tag("sensor", "2") \
		.field("value", sensorValue) \
		.time(datetime.utcnow(), WritePrecision.NS)

	write_api.write(bucket, org, point)
	client.close()
	time.sleep(1)

def onVoltageInput1_Attach(self):
	print("Attach [1]!")

def onVoltageInput1_Detach(self):
	print("Detach [1]!")

def onVoltageInput1_Error(self, code, description):
	print("Code [1]: " + ErrorEventCode.getName(code))
	print("Description [1]: " + str(description))
	print("----------")

def main():
	try:
		Log.enable(LogLevel.PHIDGET_LOG_INFO, "current_phidgetlog.log")
		#Enable server discovery to allow your program to find other Phidgets on the local network.
		Net.enableServerDiscovery(PhidgetServerType.PHIDGETSERVER_DEVICEREMOTE)

		#Create your Phidget channels
		voltageInput0 = VoltageInput()
		voltageInput1 = VoltageInput()

		#Set addressing parameters to specify which channel to open (if any)
		voltageInput0.setIsHubPortDevice(True)
		voltageInput0.setHubPort(0)
		voltageInput0.setIsRemote(True)
		voltageInput1.setIsHubPortDevice(True)
		voltageInput1.setHubPort(1)
		voltageInput1.setIsRemote(True)

		#Assign any event handlers you need before calling open so that no events are missed.
		voltageInput0.setOnSensorChangeHandler(onVoltageInput0_SensorChange)
		voltageInput0.setOnAttachHandler(onVoltageInput0_Attach)
		voltageInput0.setOnDetachHandler(onVoltageInput0_Detach)
		voltageInput0.setOnErrorHandler(onVoltageInput0_Error)
		voltageInput1.setOnSensorChangeHandler(onVoltageInput1_SensorChange)
		voltageInput1.setOnAttachHandler(onVoltageInput1_Attach)
		voltageInput1.setOnDetachHandler(onVoltageInput1_Detach)
		voltageInput1.setOnErrorHandler(onVoltageInput1_Error)

		#Open your Phidgets and wait for attachment
		voltageInput0.openWaitForAttachment(20000)
		voltageInput0.setDataInterval(1000)
		voltageInput1.openWaitForAttachment(20000)
		voltageInput1.setDataInterval(1000)

		#Do stuff with your Phidgets here or in your event handlers.
		#Set the sensor type to match the analog sensor you are using after opening the Phidget
		voltageInput0.setSensorType(VoltageSensorType.SENSOR_TYPE_3502)
		voltageInput1.setSensorType(VoltageSensorType.SENSOR_TYPE_3502)

		try:
			input("Press Enter to Stop\n")
		except (Exception, KeyboardInterrupt):
			pass

		#Close your Phidgets once the program is done.
		voltageInput0.close()
		voltageInput1.close()

	except PhidgetException as ex:
		#We will catch Phidget Exceptions here, and print the error informaiton.
		traceback.print_exc()
		print("")
		print("PhidgetException " + str(ex.code) + " (" + ex.description + "): " + ex.details)


main()
