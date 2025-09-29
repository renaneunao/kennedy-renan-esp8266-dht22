from flask import Flask, render_template, request, jsonify, redirect, url_for, flash, session
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager, login_user, logout_user, login_required, current_user
from datetime import datetime, timedelta
import os
import json
from models import db, SensorData, Device, User
import logging

# Configuração do logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Configuração do banco de dados
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = f'sqlite:///{os.path.join(basedir, "database", "sensor_data.db")}'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'dev-secret-key-change-in-production')

# Inicializar extensões
db.init_app(app)

# Configurar Flask-Login
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'
login_manager.login_message = 'Por favor, faça login para acessar esta página.'
login_manager.login_message_category = 'info'

@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))

# Criar tabelas se não existirem
with app.app_context():
    db.create_all()
    # Criar usuários padrão
    User.create_default_users()

@app.route('/login', methods=['GET', 'POST'])
def login():
    """Página de login"""
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        remember = request.form.get('remember')
        
        user = User.query.filter_by(username=username).first()
        
        if user and user.check_password(password) and user.is_active:
            login_user(user, remember=bool(remember))
            user.last_login = datetime.utcnow()
            db.session.commit()
            
            flash('Login realizado com sucesso!', 'success')
            next_page = request.args.get('next')
            return redirect(next_page) if next_page else redirect(url_for('index'))
        else:
            flash('Usuário ou senha incorretos!', 'error')
    
    return render_template('login.html')

@app.route('/logout')
@login_required
def logout():
    """Logout do usuário"""
    logout_user()
    flash('Logout realizado com sucesso!', 'info')
    return redirect(url_for('login'))

@app.route('/')
@login_required
def index():
    """Página principal com dashboard"""
    devices = Device.query.filter_by(is_active=True).all()
    return render_template('index.html', devices=devices)

@app.route('/dashboard')
@login_required
def dashboard():
    """Dashboard com gráficos em tempo real"""
    devices = Device.query.filter_by(is_active=True).all()
    return render_template('dashboard.html', devices=devices)

@app.route('/devices')
@login_required
def devices():
    """Página de gerenciamento de dispositivos"""
    devices = Device.query.all()
    return render_template('devices.html', devices=devices)

@app.route('/device/<device_id>')
@login_required
def device_detail(device_id):
    """Página de detalhes de um dispositivo específico"""
    device = Device.query.filter_by(device_id=device_id).first_or_404()
    return render_template('device_detail.html', device=device)

@app.route('/api/sensor-data', methods=['POST'])
def receive_sensor_data():
    """Endpoint para receber dados do ESP8266"""
    try:
        data = request.get_json()
        
        # Validar dados recebidos
        if not data or 'temperature' not in data or 'humidity' not in data:
            return jsonify({'status': 'error', 'message': 'Dados inválidos'}), 400
        
        # Buscar ou criar dispositivo
        device_id = data.get('device_id', 'ESP8266_001')
        device = Device.get_or_create(device_id)
        
        # Criar novo registro
        sensor_data = SensorData(
            temperature=float(data['temperature']),
            humidity=float(data['humidity']),
            device_id=device_id,
            timestamp=datetime.utcnow()
        )
        
        db.session.add(sensor_data)
        db.session.commit()
        
        logger.info(f"Dados recebidos: Temp={data['temperature']}°C, Hum={data['humidity']}%")
        
        return jsonify({
            'status': 'success',
            'message': 'Dados salvos com sucesso',
            'id': sensor_data.id
        })
        
    except Exception as e:
        logger.error(f"Erro ao salvar dados: {str(e)}")
        db.session.rollback()
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/sensor-data', methods=['GET'])
def get_sensor_data():
    """Retorna dados históricos do sensor"""
    try:
        limit = request.args.get('limit', 100, type=int)
        device_id = request.args.get('device_id')
        
        query = SensorData.query
        
        if device_id:
            query = query.filter_by(device_id=device_id)
        
        # Ordenar por timestamp decrescente e limitar resultados
        data = query.order_by(SensorData.timestamp.desc()).limit(limit).all()
        
        result = []
        for record in data:
            result.append({
                'id': record.id,
                'temperature': record.temperature,
                'humidity': record.humidity,
                'device_id': record.device_id,
                'timestamp': record.timestamp.isoformat()
            })
        
        return jsonify({
            'status': 'success',
            'data': result,
            'count': len(result)
        })
        
    except Exception as e:
        logger.error(f"Erro ao buscar dados: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/sensor-data/latest')
def get_latest_data():
    """Retorna o último registro de cada sensor"""
    try:
        # Buscar último registro de cada dispositivo
        latest_data = db.session.query(SensorData).filter(
            SensorData.id.in_(
                db.session.query(db.func.max(SensorData.id)).group_by(SensorData.device_id)
            )
        ).all()
        
        result = []
        for record in latest_data:
            result.append({
                'id': record.id,
                'temperature': record.temperature,
                'humidity': record.humidity,
                'device_id': record.device_id,
                'timestamp': record.timestamp.isoformat()
            })
        
        return jsonify({
            'status': 'success',
            'data': result
        })
        
    except Exception as e:
        logger.error(f"Erro ao buscar dados mais recentes: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/sensor-data/stats')
def get_sensor_stats():
    """Retorna estatísticas dos sensores"""
    try:
        # Estatísticas dos últimos 24 horas
        yesterday = datetime.utcnow() - timedelta(days=1)
        
        stats = db.session.query(
            db.func.avg(SensorData.temperature).label('avg_temp'),
            db.func.max(SensorData.temperature).label('max_temp'),
            db.func.min(SensorData.temperature).label('min_temp'),
            db.func.avg(SensorData.humidity).label('avg_humidity'),
            db.func.max(SensorData.humidity).label('max_humidity'),
            db.func.min(SensorData.humidity).label('min_humidity'),
            db.func.count(SensorData.id).label('total_readings')
        ).filter(SensorData.timestamp >= yesterday).first()
        
        return jsonify({
            'status': 'success',
            'stats': {
                'temperature': {
                    'average': round(stats.avg_temp, 2) if stats.avg_temp else 0,
                    'maximum': round(stats.max_temp, 2) if stats.max_temp else 0,
                    'minimum': round(stats.min_temp, 2) if stats.min_temp else 0
                },
                'humidity': {
                    'average': round(stats.avg_humidity, 2) if stats.avg_humidity else 0,
                    'maximum': round(stats.max_humidity, 2) if stats.max_humidity else 0,
                    'minimum': round(stats.min_humidity, 2) if stats.min_humidity else 0
                },
                'total_readings': stats.total_readings or 0
            }
        })
        
    except Exception as e:
        logger.error(f"Erro ao calcular estatísticas: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/devices', methods=['GET'])
def get_devices():
    """Retorna lista de dispositivos"""
    try:
        devices = Device.query.all()
        return jsonify({
            'status': 'success',
            'data': [device.to_dict() for device in devices]
        })
    except Exception as e:
        logger.error(f"Erro ao buscar dispositivos: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/devices/<device_id>', methods=['GET'])
def get_device(device_id):
    """Retorna dados de um dispositivo específico"""
    try:
        device = Device.query.filter_by(device_id=device_id).first()
        if not device:
            return jsonify({'status': 'error', 'message': 'Dispositivo não encontrado'}), 404
        
        return jsonify({
            'status': 'success',
            'data': device.to_dict()
        })
    except Exception as e:
        logger.error(f"Erro ao buscar dispositivo: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/devices/<device_id>', methods=['PUT'])
def update_device(device_id):
    """Atualiza informações de um dispositivo"""
    try:
        device = Device.query.filter_by(device_id=device_id).first()
        if not device:
            return jsonify({'status': 'error', 'message': 'Dispositivo não encontrado'}), 404
        
        data = request.get_json()
        if 'name' in data:
            device.name = data['name']
        if 'location' in data:
            device.location = data['location']
        if 'description' in data:
            device.description = data['description']
        if 'is_active' in data:
            device.is_active = data['is_active']
        
        device.updated_at = datetime.utcnow()
        db.session.commit()
        
        return jsonify({
            'status': 'success',
            'message': 'Dispositivo atualizado com sucesso',
            'data': device.to_dict()
        })
    except Exception as e:
        logger.error(f"Erro ao atualizar dispositivo: {str(e)}")
        db.session.rollback()
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/devices/<device_id>/stats')
def get_device_stats(device_id):
    """Retorna estatísticas de um dispositivo específico"""
    try:
        device = Device.query.filter_by(device_id=device_id).first()
        if not device:
            return jsonify({'status': 'error', 'message': 'Dispositivo não encontrado'}), 404
        
        hours = request.args.get('hours', 24, type=int)
        stats = device.get_stats(hours)
        
        if not stats:
            return jsonify({
                'status': 'success',
                'data': None,
                'message': 'Nenhum dado encontrado para o período especificado'
            })
        
        return jsonify({
            'status': 'success',
            'data': stats
        })
    except Exception as e:
        logger.error(f"Erro ao buscar estatísticas do dispositivo: {str(e)}")
        return jsonify({'status': 'error', 'message': 'Erro interno do servidor'}), 500

@app.route('/api/health')
def health_check():
    """Endpoint de health check"""
    try:
        # Verificar conexão com banco
        db.session.execute('SELECT 1')
        
        return jsonify({
            'status': 'healthy',
            'timestamp': datetime.utcnow().isoformat(),
            'database': 'connected'
        })
    except Exception as e:
        return jsonify({
            'status': 'unhealthy',
            'timestamp': datetime.utcnow().isoformat(),
            'error': str(e)
        }), 500

if __name__ == '__main__':
    # Criar diretório do banco se não existir
    os.makedirs(os.path.join(basedir, 'database'), exist_ok=True)
    
    # Executar aplicação
    app.run(host='0.0.0.0', port=5005, debug=True)
