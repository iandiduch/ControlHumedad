from flask import Flask, request
import json

app = Flask(__name__)

@app.route('/post', methods=['POST'])
def handle_post():
        request_body = request.data
        print("Request body:", request_body)
        return json.dumps({'resultado': True})

if __name__ == '__main__':
        app.run('0.0.0.0', port=5000)
