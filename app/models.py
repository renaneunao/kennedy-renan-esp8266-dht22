from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
from werkzeug.security import generate_password_hash, check_password_hash
from flask_login import UserMixin

db = SQLAlchemy()

class User(UserMixin, db.Model):
    """Modelo para usuários do sistema"""
    
    __tablename__ = 'users'
    
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False, comment='Nome de usuário')
    password_hash = db.Column(db.String(255), nullable=False, comment='Hash da senha')
    email = db.Column(db.String(120), unique=True, comment='Email do usuário')
    is_active = db.Column(db.Boolean, default=True, comment='Usuário ativo')
    is_admin = db.Column(db.Boolean, default=False, comment='Usuário administrador')
    created_at = db.Column(db.DateTime, default=datetime.utcnow, comment='Data de criação')
    last_login = db.Column(db.DateTime, comment='Último login')
    
    def __repr__(self):
        return f'<User {self.username}>'
    
    def set_password(self, password):
        """Define a senha do usuário"""
        self.password_hash = generate_password_hash(password)
    
    def check_password(self, password):
        """Verifica se a senha está correta"""
        return check_password_hash(self.password_hash, password)
    
    def to_dict(self):
        """Converte o objeto para dicionário"""
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'is_active': self.is_active,
            'is_admin': self.is_admin,
            'created_at': self.created_at.isoformat(),
            'last_login': self.last_login.isoformat() if self.last_login else None
        }
    
    @classmethod
    def create_default_users(cls):
        """Cria usuários padrão se não existirem"""
        # Verificar se já existem usuários
        if cls.query.count() > 0:
            return
        
        # Criar usuário renaneunao
        user1 = cls(
            username='renaneunao',
            email='renan@exemplo.com',
            is_active=True,
            is_admin=True
        )
        user1.set_password('!Senhas123')
        db.session.add(user1)
        
        # Criar usuário kennedy
        user2 = cls(
            username='kennedy',
            email='kennedy@exemplo.com',
            is_active=True,
            is_admin=False
        )
        user2.set_password('kenn$132')
        db.session.add(user2)
        
        db.session.commit()
        print("Usuários padrão criados com sucesso!")

class Device(db.Model):
    """Modelo para dispositivos/sensores"""
    
    __tablename__ = 'devices'
    
    id = db.Column(db.Integer, primary_key=True)
    device_id = db.Column(db.String(50), unique=True, nullable=False, comment='ID único do dispositivo')
    name = db.Column(db.String(100), nullable=False, comment='Nome amigável do dispositivo')
    location = db.Column(db.String(200), comment='Localização do dispositivo')
    description = db.Column(db.Text, comment='Descrição do dispositivo')
    is_active = db.Column(db.Boolean, default=True, comment='Dispositivo ativo')
    created_at = db.Column(db.DateTime, default=datetime.utcnow, comment='Data de criação')
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow, comment='Última atualização')
    
    # Relacionamento com dados do sensor
    sensor_data = db.relationship('SensorData', backref='device', lazy=True, cascade='all, delete-orphan')
    
    def __repr__(self):
        return f'<Device {self.device_id}: {self.name}>'
    
    def to_dict(self):
        """Converte o objeto para dicionário"""
        return {
            'id': self.id,
            'device_id': self.device_id,
            'name': self.name,
            'location': self.location,
            'description': self.description,
            'is_active': self.is_active,
            'created_at': self.created_at.isoformat(),
            'updated_at': self.updated_at.isoformat()
        }
    
    @classmethod
    def get_or_create(cls, device_id, name=None):
        """Busca ou cria um dispositivo"""
        device = cls.query.filter_by(device_id=device_id).first()
        if not device:
            device = cls(
                device_id=device_id,
                name=name or f"Dispositivo {device_id}",
                location="Não especificado"
            )
            db.session.add(device)
            db.session.commit()
        return device
    
    def get_latest_data(self):
        """Retorna o último dado do sensor"""
        return self.sensor_data.order_by(SensorData.timestamp.desc()).first()
    
    def get_stats(self, hours=24):
        """Retorna estatísticas do dispositivo"""
        from datetime import datetime, timedelta
        
        since = datetime.utcnow() - timedelta(hours=hours)
        records = self.sensor_data.filter(
            SensorData.timestamp >= since
        ).all()
        
        if not records:
            return None
        
        temps = [r.temperature for r in records]
        hums = [r.humidity for r in records]
        
        return {
            'device_id': self.device_id,
            'device_name': self.name,
            'period_hours': hours,
            'total_readings': len(records),
            'temperature': {
                'avg': round(sum(temps) / len(temps), 2),
                'max': round(max(temps), 2),
                'min': round(min(temps), 2)
            },
            'humidity': {
                'avg': round(sum(hums) / len(hums), 2),
                'max': round(max(hums), 2),
                'min': round(min(hums), 2)
            },
            'last_update': records[0].timestamp.isoformat() if records else None
        }

class SensorData(db.Model):
    """Modelo para dados do sensor DHT22"""
    
    __tablename__ = 'sensor_data'
    
    id = db.Column(db.Integer, primary_key=True)
    temperature = db.Column(db.Float, nullable=False, comment='Temperatura em Celsius')
    humidity = db.Column(db.Float, nullable=False, comment='Umidade em porcentagem')
    device_id = db.Column(db.String(50), db.ForeignKey('devices.device_id'), nullable=False, comment='ID do dispositivo')
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow, comment='Timestamp do registro')
    
    def __repr__(self):
        return f'<SensorData {self.device_id}: {self.temperature}°C, {self.humidity}%>'
    
    def to_dict(self):
        """Converte o objeto para dicionário"""
        return {
            'id': self.id,
            'temperature': self.temperature,
            'humidity': self.humidity,
            'device_id': self.device_id,
            'device_name': self.device.name if self.device else self.device_id,
            'timestamp': self.timestamp.isoformat()
        }
    
    @classmethod
    def get_latest_by_device(cls, device_id):
        """Retorna o último registro de um dispositivo específico"""
        return cls.query.filter_by(device_id=device_id).order_by(cls.timestamp.desc()).first()
    
    @classmethod
    def get_stats_by_device(cls, device_id, hours=24):
        """Retorna estatísticas de um dispositivo nas últimas N horas"""
        from datetime import datetime, timedelta
        
        since = datetime.utcnow() - timedelta(hours=hours)
        records = cls.query.filter(
            cls.device_id == device_id,
            cls.timestamp >= since
        ).all()
        
        if not records:
            return None
        
        temps = [r.temperature for r in records]
        hums = [r.humidity for r in records]
        
        return {
            'device_id': device_id,
            'period_hours': hours,
            'total_readings': len(records),
            'temperature': {
                'avg': round(sum(temps) / len(temps), 2),
                'max': round(max(temps), 2),
                'min': round(min(temps), 2)
            },
            'humidity': {
                'avg': round(sum(hums) / len(hums), 2),
                'max': round(max(hums), 2),
                'min': round(min(hums), 2)
            },
            'last_update': records[0].timestamp.isoformat()
        }
