from flask import Flask, request, jsonify, redirect, url_for, render_template, session
from flask_cors import CORS
import requests
# import json
import paho.mqtt.client as mqtt

import datetime

mqtt_host = "192.168.182.37"
host = "192.168.182.37"
server_address = f"http://{host}:3000"
port = 1883
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    client.subscribe("authen", qos=2)

def on_message(client, userdata, msg):
    try:
        print("fn called")
        # print(msg.topic + " " + str(msg.payload))
        # try:
        received_msg = msg.payload.decode("utf-8")
        print(f"Received msg: {received_msg}")
        get_json = requests.get(server_address + '/users')
        res = get_json.json()
        if res:
            for i in range(len(res)):
                # print(f"Loop: {i}")
                if res[i]['uid'] == received_msg:
                    print(res[i])
                    if res[i]['authorize'] == True:
                        get_room_status = requests.get(server_address + '/room_status')
                        room_status = get_room_status.json() 
                        if room_status['status'] == 'Unoccupied':
                            data = {"status": "Occupied", "occupied_by": res[i]['uid']}
                            edit_json = requests.patch(server_address + '/room_status', json=data)
                            print(f"Edit to True: {edit_json.status_code}")
                            client.publish("door_state", "1", qos=2)

                            timestamp = datetime.datetime.now().strftime("%d-%m-%Y, %H:%M:%S")
                            data = {"open": timestamp, "close": "-", "opened_by": res[i]['uid'], "closed_by": "-"}
                            post_log = requests.post(server_address + '/log', json=data)

                        else:
                            data = {"status": "Unoccupied", "occupied_by": "N/A"}
                            edit_json = requests.patch(server_address + '/room_status', json=data)
                            print(f"Edit to False: {edit_json.status_code}")
                            client.publish("door_state", "0", qos=2)

                            timestamp = datetime.datetime.now().strftime("%d-%m-%Y, %H:%M:%S")
                            data = {"close": timestamp, "closed_by": res[i]['uid']}
                            log = requests.get(server_address + '/log')
                            log = log.json()
                            update_log = requests.patch(server_address + f'/log/{log[-1]["id"]}', json=data)
                    else:
                        client.publish("door_state", "-1", qos=2)
                    break
    except Exception as e:
        print(e)
        client.publish("door_state", "-1", qos=2)
        exec(open("app.py").read())

client.on_connect = on_connect
client.on_message = on_message
client.connect(mqtt_host, port)

app = Flask(__name__)
CORS(app)
@app.route('/')
def home():
    if 'uid' in session:
        return f'Logged in as {session["uid"]}'
    return render_template('login.html')

@app.route('/loginform')
def loginform():
    # Your home route logic here
    return render_template('login.html')

@app.route('/controlDoor')
def controlDoor():
    return render_template('controlDoor.html')

@app.route("/tables")
def tables():
    return render_template("tables.html")

@app.route("/loglast")
def log_last():
    try:
        headers = {'Accept': 'application/json'}
        r = requests.get(f'{server_address}/log', headers=headers)
        r = r.json()
        r = r[-1]
        return jsonify(r)

    except Exception as e:
        print(f'Error: {e}')
        return jsonify({'message': 'Internal Server Error'}), 500
    
@app.route("/log")
def log():
    try:
        headers = {'Accept': 'application/json'}
        r = requests.get(f'{server_address}/log', headers=headers)

        return r.json()


    except Exception as e:
        print(f'Error: {e}')
        return jsonify({'message': 'Internal Server Error'}), 500
    
@app.route("/door_status")
def get_door_status():
    try:
        headers = {'Accept': 'application/json'}
        r = requests.get(f'{server_address}/room_status', headers=headers)
        return r.json()

    except Exception as e:
        print(f'Error: {e}')
        return jsonify({'message': 'Internal Server Error'}), 500


@app.route('/login', methods=['POST'])
def login():
    try:
        if request.method == 'POST':
            uid = request.json.get('uid')
            password = request.json.get('password')

            print(f'Received data: uid={uid}, password={password}')

            # Simulating a request to your JSON server
            response = requests.get(f'{server_address}/users')
            users = response.json()

            print(users)

            # Check if the user exists in the simulated JSON data
            user = next((u for u in users if u.get('uid') == uid and u.get('password') == password), None)
            print(user)

            if user:
                if user.get('authorize') == True:
                    return jsonify({'message': 'Login successful', 'uid': user['uid'], 'fname': user['fname'], 'lname': user['lname']}), 200
                elif user.get('authorize') == False:
                    return jsonify({'message': 'Insufficient privileges'}), 401
            return jsonify({'message': 'Invalid credentials'}), 401
        else:
            return jsonify({'message': 'Method not allowed'}), 405

    except Exception as e:
        print(f'Error: {e}')
        return jsonify({'message': 'Internal Server Error'}), 500

@app.route('/logout')
def logout():
    session.pop('uid', None)
    return redirect(url_for('loginform'))

@app.route('/toggle_door', methods=['POST'])
def toggle_door():
    uid = request.json.get('uid')
    state = request.json.get('state')
    json = requests.get(f'{server_address}/users')
    json = json.json()
    print(uid)
    if json:
        for i in range(len(json)):
            if json[i]['uid'] == uid:
                print(json[i])
                if json[i]['authorize'] == True:
                    get_room_status = requests.get(server_address + '/room_status')
                    room_status = get_room_status.json() 
                    if room_status['status'] == 'Unoccupied' and state == True:
                        data = {"status": "Occupied", "occupied_by": json[i]['uid']}
                        edit_json = requests.patch(server_address + '/room_status', json=data)
                        print(f"Edit to True: {edit_json.status_code}")
                        client.publish("door_state", "1", qos=2)

                        timestamp = datetime.datetime.now().strftime("%d-%m-%Y, %H:%M:%S")
                        data = {"open": timestamp, "close": "-", "opened_by": json[i]['uid'], "closed_by": "-"}
                        post_log = requests.post(server_address + '/log', json=data)
                    elif room_status['status'] == 'Occupied' and state == False:
                        if room_status["occupied_by"] == uid:
                            data = {"status": "Unoccupied", "occupied_by": "N/A"}
                            edit_json = requests.patch(server_address + '/room_status', json=data)
                            print(f"Edit to False: {edit_json.status_code}")
                            client.publish("door_state", "0", qos=2)

                            timestamp = datetime.datetime.now().strftime("%d-%m-%Y, %H:%M:%S")
                            data = {"close": timestamp, "closed_by": json[i]['uid']}
                            log = requests.get(server_address + '/log')
                            log = log.json()
                            update_log = requests.patch(server_address + f'/log/{log[-1]["id"]}', json=data)
                        else:
                            return jsonify({'message': f'{room_status["occupied_by"]} is occupied the room!'}), 400
                    else:
                        if room_status['status'] == 'Occupied' and state == True:
                            return jsonify({'door_state': 'door is opened'}), 400
                        elif room_status['status'] == 'Unoccupied' and state == False:
                            return jsonify({'door_state': 'door is closed'}), 400
    return jsonify({'message': 'OK'}), 200


if __name__ == "__main__":
    client.loop_start()
    # app.run(host= '0.0.0.0', debug=True)
    app.run(host='0.0.0.0', debug=False)