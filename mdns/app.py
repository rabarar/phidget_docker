import time

from flask import Flask 


app = Flask(__name__)


@app.route('/')
def hello():
    return "hello there!"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8888, debug=True)

