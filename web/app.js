function card(bed, patient) {
  return `<div class="card">
    <div class="cardTop"><h2>${bed}: ${patient.name}</h2><span class="badge ${patient.status}">${patient.status}</span></div>
    <div class="row"><span>Heart Rate</span><b>${patient.hr} bpm</b></div>
    <div class="row"><span>Oxygen Level</span><b>${patient.spo2}%</b></div>
    <div class="row"><span>Temperature</span><b>${patient.temp.toFixed(1)} C</b></div>
    <div class="row"><span>Health Score</span><b>${patient.score}/100</b></div>
    <div class="row"><span>Fall Detection</span><b>${patient.fall ? 'YES' : 'NO'}</b></div>
  </div>`;
}

function drawGraph(hr, spo2, temp) {
  const canvas = document.getElementById('graph');
  const ctx = canvas.getContext('2d');
  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);
  ctx.fillStyle = '#fbfdff';
  ctx.fillRect(0, 0, w, h);
  ctx.strokeStyle = '#e2e8f0';
  ctx.lineWidth = 1;
  ctx.font = '12px Arial';
  ctx.fillStyle = '#64748b';
  for (let i = 0; i < 6; i++) {
    const y = 34 + i * 42;
    ctx.beginPath(); ctx.moveTo(48, y); ctx.lineTo(w - 24, y); ctx.stroke();
  }

  function line(samples, color, min, max) {
    ctx.strokeStyle = color;
    ctx.lineWidth = 3;
    ctx.beginPath();
    const points = samples.map((value, index) => {
      const x = 58 + index * ((w - 95) / 9);
      let y = value <= 0 ? h - 36 : h - 36 - ((value - min) / (max - min)) * (h - 78);
      y = Math.max(24, Math.min(h - 32, y));
      return { x, y };
    });
    points.forEach(({ x, y }, index) => index ? ctx.lineTo(x, y) : ctx.moveTo(x, y));
    ctx.stroke();
    ctx.fillStyle = color;
    points.forEach(({ x, y }) => {
      ctx.beginPath(); ctx.arc(x, y, 3, 0, 2 * Math.PI); ctx.fill();
    });
  }

  line(hr, '#2563eb', 40, 140);
  line(spo2, '#059669', 80, 100);
  line(temp, '#d97706', 25, 42);
}

async function refresh() {
  try {
    const response = await fetch('/data');
    if (!response.ok) throw new Error('Non-OK response');
    const data = await response.json();
    const patients = [data.bed1, data.bed2, data.bed3];
    document.getElementById('cards').innerHTML =
      card('Bed 1', data.bed1) + card('Bed 2', data.bed2) + card('Bed 3', data.bed3);

    document.getElementById('crit').textContent = patients.filter(p => p.status === 'CRITICAL').length;
    document.getElementById('warn').textContent = patients.filter(p => p.status === 'WARNING').length;
    document.getElementById('norm').textContent = patients.filter(p => p.status === 'NORMAL').length;

    const alerted = patients.find(p => p.status === 'CRITICAL' || p.status === 'WARNING');
    const banner = document.getElementById('banner');
    banner.style.display = alerted ? 'block' : 'none';
    if (alerted) banner.textContent = 'Patient alert: ' + alerted.name + ' requires attention';

    drawGraph(data.hrHistory, data.spo2History, data.tempHistory);
    document.getElementById('foot').textContent =
      'ESP32 IP: ' + data.ip + ' | WiFi: ' + data.wifi +
      ' | Last update: ' + data.timestamp +
      (data.baseline ? ' | Baseline ready' : ' | Calibrating baseline');
  } catch (error) {
    document.getElementById('foot').textContent = 'Dashboard error: /data not responding';
  }
}

setInterval(refresh, 2000);
refresh();
