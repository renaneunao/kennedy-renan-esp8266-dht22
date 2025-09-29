// ESP8266 DHT22 Monitor - JavaScript Application

class SensorMonitor {
    constructor() {
        this.apiBaseUrl = '';
        this.updateInterval = 30000; // 30 segundos
        this.isAutoRefresh = false;
        this.charts = {};
        this.init();
    }

    init() {
        console.log('Inicializando Sensor Monitor...');
        this.setupEventListeners();
        this.loadInitialData();
    }

    setupEventListeners() {
        // Auto-refresh toggle
        const refreshBtn = document.getElementById('refresh-btn');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', () => this.toggleAutoRefresh());
        }

        // Time range selector
        const timeRange = document.getElementById('timeRange');
        if (timeRange) {
            timeRange.addEventListener('change', () => this.updateCharts());
        }

        // Device selector
        const deviceSelect = document.getElementById('deviceSelect');
        if (deviceSelect) {
            deviceSelect.addEventListener('change', () => this.updateCharts());
        }
    }

    async loadInitialData() {
        try {
            await this.updateCurrentData();
            await this.updateStats();
            await this.updateCharts();
        } catch (error) {
            console.error('Erro ao carregar dados iniciais:', error);
            this.showError('Erro ao carregar dados iniciais');
        }
    }

    async fetchData(url) {
        try {
            const response = await fetch(url);
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return await response.json();
        } catch (error) {
            console.error('Erro na requisição:', error);
            throw error;
        }
    }

    async updateCurrentData() {
        try {
            const data = await this.fetchData('/api/sensor-data/latest');
            
            if (data.status === 'success' && data.data.length > 0) {
                const latest = data.data[0];
                this.updateCurrentDisplay(latest);
            } else {
                this.showNoData();
            }
        } catch (error) {
            console.error('Erro ao atualizar dados atuais:', error);
            this.showError('Erro de conexão');
        }
    }

    updateCurrentDisplay(data) {
        // Atualizar elementos da página inicial
        const tempElement = document.getElementById('current-temp');
        const humidityElement = document.getElementById('current-humidity');
        const deviceElement = document.getElementById('device-id');
        const lastUpdateElement = document.getElementById('last-update');
        const timeAgoElement = document.getElementById('time-ago');

        if (tempElement) {
            tempElement.textContent = `${data.temperature.toFixed(1)}°C`;
            tempElement.classList.add('data-update');
            setTimeout(() => tempElement.classList.remove('data-update'), 500);
        }

        if (humidityElement) {
            humidityElement.textContent = `${data.humidity.toFixed(1)}%`;
            humidityElement.classList.add('data-update');
            setTimeout(() => humidityElement.classList.remove('data-update'), 500);
        }

        if (deviceElement) {
            deviceElement.textContent = data.device_id;
        }

        if (lastUpdateElement) {
            const timestamp = new Date(data.timestamp);
            lastUpdateElement.textContent = timestamp.toLocaleTimeString('pt-BR');
        }

        if (timeAgoElement) {
            timeAgoElement.textContent = this.getTimeAgo(new Date(data.timestamp));
        }

        // Atualizar timestamps dos cards
        const tempTimestamp = document.getElementById('temp-timestamp');
        const humidityTimestamp = document.getElementById('humidity-timestamp');
        
        if (tempTimestamp) {
            tempTimestamp.textContent = `Atualizado: ${new Date(data.timestamp).toLocaleString('pt-BR')}`;
        }
        
        if (humidityTimestamp) {
            humidityTimestamp.textContent = `Atualizado: ${new Date(data.timestamp).toLocaleString('pt-BR')}`;
        }

        // Atualizar status do dispositivo
        const deviceStatus = document.getElementById('device-status');
        if (deviceStatus) {
            deviceStatus.textContent = 'Online';
            deviceStatus.className = 'text-success';
        }
    }

    async updateStats() {
        try {
            const data = await this.fetchData('/api/sensor-data/stats');
            
            if (data.status === 'success') {
                this.updateStatsDisplay(data.stats);
            }
        } catch (error) {
            console.error('Erro ao atualizar estatísticas:', error);
        }
    }

    updateStatsDisplay(stats) {
        // Atualizar estatísticas de temperatura
        const avgTemp = document.getElementById('avg-temp');
        const maxTemp = document.getElementById('max-temp');
        const minTemp = document.getElementById('min-temp');

        if (avgTemp) avgTemp.textContent = `${stats.temperature.average}°C`;
        if (maxTemp) maxTemp.textContent = `${stats.temperature.maximum}°C`;
        if (minTemp) minTemp.textContent = `${stats.temperature.minimum}°C`;

        // Atualizar estatísticas de umidade
        const avgHumidity = document.getElementById('avg-humidity');
        const maxHumidity = document.getElementById('max-humidity');
        const minHumidity = document.getElementById('min-humidity');

        if (avgHumidity) avgHumidity.textContent = `${stats.humidity.average}%`;
        if (maxHumidity) maxHumidity.textContent = `${stats.humidity.maximum}%`;
        if (minHumidity) minHumidity.textContent = `${stats.humidity.minimum}%`;

        // Atualizar total de leituras
        const totalReadings = document.getElementById('total-readings');
        if (totalReadings) {
            totalReadings.textContent = stats.total_readings;
        }
    }

    async updateCharts() {
        try {
            const timeRange = document.getElementById('timeRange')?.value || 24;
            const deviceId = document.getElementById('deviceSelect')?.value || '';
            
            let url = `/api/sensor-data?limit=${timeRange * 60}`;
            if (deviceId) {
                url += `&device_id=${deviceId}`;
            }

            const data = await this.fetchData(url);
            
            if (data.status === 'success') {
                this.updateMainChart(data.data);
                this.updateHumidityChart(data.data);
                this.updateDataTable(data.data);
            }
        } catch (error) {
            console.error('Erro ao atualizar gráficos:', error);
        }
    }

    updateMainChart(data) {
        if (!this.charts.main) return;

        const labels = data.map(item => new Date(item.timestamp).toLocaleTimeString('pt-BR'));
        const temperatures = data.map(item => item.temperature);
        const humidities = data.map(item => item.humidity);

        this.charts.main.data.labels = labels;
        this.charts.main.data.datasets[0].data = temperatures;
        this.charts.main.data.datasets[1].data = humidities;
        this.charts.main.update();
    }

    updateHumidityChart(data) {
        if (!this.charts.humidity) return;

        const humidities = data.map(item => item.humidity);
        const low = humidities.filter(h => h < 30).length;
        const normal = humidities.filter(h => h >= 30 && h <= 70).length;
        const high = humidities.filter(h => h > 70).length;

        this.charts.humidity.data.datasets[0].data = [low, normal, high];
        this.charts.humidity.update();
    }

    updateDataTable(data) {
        const tbody = document.getElementById('dataTableBody');
        if (!tbody) return;

        tbody.innerHTML = '';

        data.slice(0, 10).forEach(item => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${new Date(item.timestamp).toLocaleString('pt-BR')}</td>
                <td>${item.device_id}</td>
                <td>${item.temperature.toFixed(1)}°C</td>
                <td>${item.humidity.toFixed(1)}%</td>
                <td><span class="badge bg-success">OK</span></td>
            `;
            tbody.appendChild(row);
        });
    }

    toggleAutoRefresh() {
        const icon = document.getElementById('refresh-icon');
        const text = document.getElementById('refresh-text');

        if (this.isAutoRefresh) {
            clearInterval(this.refreshInterval);
            if (icon) icon.className = 'fas fa-play me-1';
            if (text) text.textContent = 'Auto Refresh';
            this.isAutoRefresh = false;
        } else {
            this.refreshInterval = setInterval(() => {
                this.updateCharts();
                this.updateCurrentData();
            }, this.updateInterval);
            if (icon) icon.className = 'fas fa-pause me-1';
            if (text) text.textContent = 'Pausar';
            this.isAutoRefresh = true;
        }
    }

    getTimeAgo(date) {
        const now = new Date();
        const diff = now - date;
        const minutes = Math.floor(diff / 60000);

        if (minutes < 1) return 'agora';
        if (minutes < 60) return `${minutes} min atrás`;

        const hours = Math.floor(minutes / 60);
        if (hours < 24) return `${hours}h atrás`;

        const days = Math.floor(hours / 24);
        return `${days}d atrás`;
    }

    showNoData() {
        const tempElement = document.getElementById('current-temp');
        const humidityElement = document.getElementById('current-humidity');
        const deviceStatus = document.getElementById('device-status');

        if (tempElement) tempElement.textContent = '--°C';
        if (humidityElement) humidityElement.textContent = '--%';
        if (deviceStatus) {
            deviceStatus.textContent = 'Sem dados';
            deviceStatus.className = 'text-warning';
        }
    }

    showError(message) {
        console.error(message);
        // Implementar notificação de erro se necessário
    }

    // Método para registrar gráficos
    registerChart(name, chart) {
        this.charts[name] = chart;
    }
}

// Inicializar aplicação quando o DOM estiver carregado
document.addEventListener('DOMContentLoaded', function() {
    window.sensorMonitor = new SensorMonitor();
});

// Funções utilitárias globais
function formatTemperature(temp) {
    return `${temp.toFixed(1)}°C`;
}

function formatHumidity(humidity) {
    return `${humidity.toFixed(1)}%`;
}

function formatTimestamp(timestamp) {
    return new Date(timestamp).toLocaleString('pt-BR');
}

// Exportar para uso global
window.SensorMonitor = SensorMonitor;

