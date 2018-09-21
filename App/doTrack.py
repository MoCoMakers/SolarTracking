from flask import Flask
from flask import render_template, request, url_for

import cgitb
cgitb.enable()

import sun_tracking as suntr

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
		result = suntr.get_azimuth(start_time, latitude, longitude, current_height)
		return "Result: "+str(result)

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
		return "Command was "+command+"\nResult was: "+str(current_azimuth)
	else:
		return "Command not found"