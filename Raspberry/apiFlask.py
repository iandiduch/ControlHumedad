from flask import Flask, request, jsonify
from flask_cors import CORS
from config import Config
from models import db, HumidityData
import json

app = Flask(__name__)
CORS(app)
app.config.from_object(Config)
db.init_app(app)

with app.app_context():
    db.create_all()


@app.route('/api/sensor/last', methods=['GET'])
def get_sensor_data_last():
    # Obtener el último registro de la base de datos
    latest_data = HumidityData.query.order_by(HumidityData.timestamp.desc()).first()
    
    if latest_data is None:
        return jsonify({'mensaje': 'No hay datos disponibles'}), 404
    
    data_dict = {
        'id': latest_data.id,
        'humidity1': latest_data.humidity1,
        'humidity2': latest_data.humidity2,
        'humidity3': latest_data.humidity3,
        'timestamp': latest_data.timestamp.strftime('%Y-%m-%d %H:%M:%S')
    }
    
    return jsonify(data_dict)
    
@app.route('/api/sensor/all', methods=['GET'])
def get_sensor_data_all():
    sensor_data = []
    # Obtener todos los registros de la base de datos
    all_data = HumidityData.query.all()
    
    # Convertir cada registro a un diccionario
    for data in all_data:
        data_dict = {
            'id': data.id,
            'humidity1': data.humidity1,
            'humidity2': data.humidity2,
            'humidity3': data.humidity3,
            'timestamp': data.timestamp.strftime('%Y-%m-%d %H:%M:%S')
        }
        sensor_data.append(data_dict)
    
    return jsonify(sensor_data)

@app.route('/post', methods=['POST'])
def handle_post():
    request_body = request.get_json()
    
    if not request_body:
        return jsonify({'resultado': False, 'mensaje': 'No se recibió JSON válido'}), 400
    
    humidity1 = request_body.get('humidity1')
    humidity2 = request_body.get('humidity2')
    humidity3 = request_body.get('humidity3')
    
    if humidity1 is None or humidity2 is None or humidity3 is None:
        return jsonify({'resultado': False, 'mensaje': 'Faltan datos de humedad'}), 400
    
    new_data = HumidityData(humidity1=humidity1, humidity2=humidity2, humidity3=humidity3)
    db.session.add(new_data)
    db.session.commit()
    
    print(f"Request body: {request_body}")
    return jsonify({'resultado': True})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)


