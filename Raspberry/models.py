from flask_sqlalchemy import SQLAlchemy

db = SQLAlchemy()

class HumidityData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    humidity1 = db.Column(db.Float, nullable=False)
    humidity2 = db.Column(db.Float, nullable=False)
    humidity3 = db.Column(db.Float, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=db.func.current_timestamp())
