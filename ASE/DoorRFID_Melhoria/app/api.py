# flask api 
import json
from flask import Flask, jsonify, request,render_template
from flask_cors import CORS
import datetime
import requests
import socket
import matplotlib.pyplot as plt
rfidip = ""


# VARIABLES
app = Flask(__name__)
JSONFILE = 'data.json'

@app.route('/', methods=['GET'])
def home():
    return "<h1>API</h1><p>This RFIDsite is a prototype API for the users and logs.</p>"

@app.route('/users', methods=['GET'])
def get_users():
    if 'rfid' in request.args:
        # verify if the rfid is in the users
        rfid = request.args['rfid']
        users = getUsers()
        results = []
        response = "false",300
        for user in users:
            if user['rfid'] == rfid:
                response = "true",200
        return response
    users = getUsers()
    return jsonify(users)

@app.route('/users', methods=['POST'])
def post_users():
    # http://ip:5000/users?rfid=123456789
    users = getUsers()
    logs = getLogs()
    rfid = request.args.get('rfid')
    users.append({'rfid': rfid, 'day': datetime.datetime.now().strftime("%d-%m-%Y"), 'hour': datetime.datetime.now().strftime("%Hh%M")})
    with open(JSONFILE, 'w') as outfile:
        json.dump({'users': users, 'logs': logs}, outfile)
    return jsonify(users)

@app.route('/users', methods=['DELETE'])
def delete_users():
    # http://ip:5000/users?rfid=123456789
    users = getUsers()
    logs = getLogs()
    rfid = request.args.get('rfid')
    users = [user for user in users if user['rfid'] != rfid]
    with open(JSONFILE, 'w') as outfile:
        json.dump({'users': users, 'logs': logs}, outfile)
    return jsonify(users)

@app.route('/logs', methods=['GET'])
def get_logs():
    return jsonify(getLogs())

@app.route('/logs', methods=['POST'])
def post_logs():
    # http://ip:5000/users?data=12-05-2000&hora=18:29&status=Allow&rfid=123456789
    users = getUsers()
    logs = getLogs()
    data = request.args.get('data')
    hora = request.args.get('hora')
    status = request.args.get('status')
    rfid = request.args.get('rfid')
    logs.append({'data': data, 'hora': hora, 'status': status, 'rfid': rfid})
    with open(JSONFILE, 'w') as outfile:
        json.dump({'users': users, 'logs': logs}, outfile)
    return jsonify(logs)

# auxiliar function
def getJson(file):
    with open(file) as json_file:
        data = json.load(json_file)
    return data

def getUsers():
    return getJson(JSONFILE)['users']

def getLogs():
    return getJson(JSONFILE)['logs']

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)