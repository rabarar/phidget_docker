from Phidget22.PhidgetException import *
from Phidget22.Phidget import *
from Phidget22.Net import *
from Phidget22.Devices.Log import *
from Phidget22.LogLevel import *
from Phidget22.Devices.TemperatureSensor import *
import traceback
import time

from datetime import datetime
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# You can generate an API token from the "API Tokens Tab" in the UI
token = "oVCPm7IyJn79ILFwHr7DPKrkEvVbc3zMVcaqIGXLl6-RnneIHwe9i2KbX_8MTQ5Q5rmGT-l-auXE80Km4QH1tg=="
org = "Baruch"
bucket = "sensors"


#Declare any event handlers here. These will be called every time the associated event occurs.

def onTemperatureChange(self, temperature):
	print("Temperature: " + str(temperature) + "channel: " + str(self.getChannel()) + " HubPort: " +  str(self.getHubPort()))
	client=InfluxDBClient(url="http://localhost:8186", token=token, org=org)
	write_api = client.write_api(write_options=SYNCHRONOUS)
	point = Point("temp") \
		.tag("sensor", str(self.getHubPort())) \
		.field("value", temperature) \
		.time(datetime.utcnow(), WritePrecision.NS)

	write_api.write(bucket, org, point)
	client.close()


def onAttach(self):
	print("Attach!")

def onDetach(self):
	print("Detach!")

def onError(self, code, description):
	print("Code: " + ErrorEventCode.getName(code))
	print("Description: " + str(description))
	print("----------")

def main():
	try:
		Log.enable(LogLevel.PHIDGET_LOG_INFO, "temp_phidgetlog.log")
		#Enable server discovery to allow your program to find other Phidgets on the local network.
		Net.enableServerDiscovery(PhidgetServerType.PHIDGETSERVER_DEVICEREMOTE)

		#Create your Phidget channels
		temperatureSensor0 = TemperatureSensor()
		temperatureSensor1 = TemperatureSensor()

		#Set addressing parameters to specify which channel to open (if any)
		temperatureSensor0.setHubPort(4)
		temperatureSensor0.setIsRemote(True)

		temperatureSensor0.setHubPort(5)
		temperatureSensor0.setIsRemote(True)

		#Assign any event handlers you need before calling open so that no events are missed.
		temperatureSensor0.setOnTemperatureChangeHandler(onTemperatureChange)
		temperatureSensor0.setOnAttachHandler(onAttach)
		temperatureSensor0.setOnDetachHandler(onDetach)
		temperatureSensor0.setOnErrorHandler(onError)

		temperatureSensor1.setOnTemperatureChangeHandler(onTemperatureChange)
		temperatureSensor1.setOnAttachHandler(onAttach)
		temperatureSensor1.setOnDetachHandler(onDetach)
		temperatureSensor1.setOnErrorHandler(onError)

		#Open your Phidgets and wait for attachment
		temperatureSensor0.openWaitForAttachment(20000)
		temperatureSensor0.setDataInterval(1)

		temperatureSensor1.openWaitForAttachment(20000)
		temperatureSensor1.setDataInterval(1)

		#Do stuff with your Phidgets here or in your event handlers.

		try:
			input("Press Enter to Stop\n")
		except (Exception, KeyboardInterrupt):
			pass

		#Close your Phidgets once the program is done.
		temperatureSensor0.close()
		temperatureSensor1.close()

	except PhidgetException as ex:
		#We will catch Phidget Exceptions here, and print the error informaiton.
		traceback.print_exc()
		print("")
		print("PhidgetException " + str(ex.code) + " (" + ex.description + "): " + ex.details)


main()
