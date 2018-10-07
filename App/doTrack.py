from flask import Flask
from flask import render_template, request, url_for

import cgitb
cgitb.enable()

import sun_tracking as suntr

from datetime import datetime, timedelta
from geomag import geomag
import inspect
import os

app = Flask(__name__, static_url_path='/static')

@app.route("/", methods=['GET', 'POST'])
def hello(name= None):
	if request.method == "POST":
		try:
			longitude = request.form['longitude']
			latitude = request.form['latitude']
			start_time = request.form['result-start-time']
			end_time = request.form['result-end-time']
			interval = request.form['interval']
			#return "long: "+str(longitude)+" and lat:"+str(latitude)+" start time:"+str(start_time)+" end_time"+str(end_time)+" inteval:"+str(interval)
			
		except:
			return "Input data was not formatted properly."
			
		current_height=0
		longitude=float(longitude)
		latitude=float(latitude)
		
		delta=0
		start_datetime=datetime.strptime(start_time,"%Y-%m-%d %H:%M")
		end_datetime=datetime.strptime(end_time,"%Y-%m-%d %H:%M")
		new_datetime = start_datetime +timedelta(0,int(interval)) # days, seconds, then other fields.
		all_values=""
		list_of_dates=[]
		while(new_datetime<end_datetime):
			delta=delta+int(interval)
			new_date=datetime.strftime(new_datetime, "%Y-%m-%d %H:%M:%S")
			list_of_dates.append(new_date)
			new_datetime=start_datetime+timedelta(0,delta) #increment the time by seconds
		
		path = os.path.dirname(inspect.getfile(geomag))
		COF_File= os.path.join(path,"WMM.COF")
		
		gm = geomag.GeoMag(COF_File)
		
		
		total=len(list_of_dates)-1
		for index in range(0,len(list_of_dates)):
			print "At "+str(((index*1.0)/total)*100)+"%"
			current_date=list_of_dates[index]
			result_alt, result_azi = suntr.get_azimuth(current_date, latitude, longitude, current_height)
			
			#Get magnetic delictionation
			mag = gm.GeoMag(latitude,longitude)
			declination = mag.dec # -6.1335150785195536
			
			current_value= current_date+","+str(result_alt)+","+str(result_azi)+","+str(declination)
			all_values=all_values+current_value+"<br>"
			
			

		
		print "start_time: "+str(start_time)
		result = suntr.get_azimuth(start_time, latitude, longitude, current_height)
		
		return str(all_values)

	else:
		return render_template('grab_data.html', name=name)
	
@app.route("/command/<command>")
def run_command(command=None):
	if command=="go_to_sun":
		obstime = "2013-09-21 16:00:00"
		latitude = 34.4900
		longitude = -104.221800
		current_height= 40
		current_azimuth = suntr.get_azimuth(obstime, latitude, longitude, current_height)
		
		#Get Magnetic Declination (angle devation from True )
		
		path = os.path.dirname(inspect.getfile(geomag))
		COF_File= os.path.join(path,"WMM.COF")
		
		gm = geomag.GeoMag(COF_File)
		
		mag = gm.GeoMag(latitude,longitude)
		
		declination = mag.dec # -6.1335150785195536


		return "Command was "+command+"\nResult was: "+str(current_azimuth)+" declicnation:"+str(declination)
	else:
		return "Command not found"