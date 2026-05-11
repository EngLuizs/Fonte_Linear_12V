#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>FLM Systems - Fonte 12V</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    font-family: 'Times New Roman', serif;
    background: #121a2c;
    color: white;
    text-align: center;
  }

  /* ── TELAS ── */
  #main-screen { display: flex; flex-direction: column; min-height: 100vh; }
  #chart-screen { display: none; flex-direction: column; min-height: 100vh; }

  /* ── TELA PRINCIPAL ── */
  .header {
    padding: 20px;
    font-size: 30px;
    font-weight: bold;
    margin-top: 20px;
  }

  .container { padding: 10px; }

  .card {
    background: #1e293b;
    border-radius: 15px;
    padding: 20px;
    margin: 10px;
    box-shadow: 0 4px 15px rgba(0,0,0,0.4);
  }

  .value {
    font-size: 32px;
    color: #22c55e;
    margin-top: 10px;
  }

  .label { font-size: 14px; color: #94a3b8; }

  .button {
    margin-top: 15px;
    padding: 12px;
    border-radius: 10px;
    border: none;
    background: #2563eb;
    color: white;
    font-size: 16px;
    width: 80%;
    cursor: pointer;
  }

  img.logo { width: 100px; margin-top: 10px; }

  .footer {
    font-size: 12px;
    font-weight: bold;
    color: #64748b;
    padding: 20px;
  }
  .footer img { width: 240px; }

  /* ── TELA DE GRÁFICOS ── */
  .chart-header {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 15px 16px;
    background: #0f172a;
    border-bottom: 1px solid #1e293b;
  }

  .back-btn {
    background: #334155;
    border: none;
    color: white;
    border-radius: 8px;
    padding: 9px 14px;
    font-size: 15px;
    cursor: pointer;
  }

  .chart-header-title {
    font-size: 20px;
    font-weight: bold;
    flex: 1;
  }

  .charts-wrapper { padding: 12px; flex: 1; }

  .chart-card {
    background: #1e293b;
    border-radius: 15px;
    padding: 14px;
    margin-bottom: 14px;
    box-shadow: 0 4px 15px rgba(0,0,0,0.4);
    text-align: left;
  }

  .chart-card-label {
    font-size: 12px;
    color: #94a3b8;
    text-transform: uppercase;
    letter-spacing: 0.06em;
    margin-bottom: 4px;
  }

  .chart-live-value {
    font-size: 28px;
    font-weight: bold;
    margin-bottom: 10px;
  }

  .cv-color { color: #38bdf8; }
  .ci-color { color: #fb923c; }
  .cp-color { color: #22c55e; }

  canvas {
    width: 100%;
    height: 110px;
    display: block;
    border-radius: 6px;
  }
</style>
</head>
<body>

<!-- ══════════════ TELA PRINCIPAL ══════════════ -->
<div id="main-screen">
  <div class="header">Power Supply Monitor</div>

  <div class="container">
    <img class="logo" src="/fonte.png">

    <div class="card">
      <div class="label">Tensão</div>
      <div class="value" id="v">-- V</div>
    </div>

    <div class="card">
      <div class="label">Corrente</div>
      <div class="value" id="i">-- A</div>
    </div>

    <div class="card">
      <div class="label">Potência</div>
      <div class="value" id="p">-- W</div>
    </div>

    <button class="button" onclick="irParaGraficos()">&#128202; Mostrar Gráfico</button>
  </div>

  <div class="footer">
    &copy; 2026 FLM Systems. Todos os direitos reservados.
    <br>Developers: Luiz H., Felipe C., Vinicius S., Maycon B., Luis Z.<br>
    <br><img src="/logo.png"><br>
  </div>
</div>

<!-- ══════════════ TELA DE GRÁFICOS ══════════════ -->
<div id="chart-screen">
  <div class="chart-header">
    <button class="back-btn" onclick="voltarParaMain()">&#8592; Voltar</button>
    <span class="chart-header-title">Gráficos em Tempo Real</span>
  </div>

  <div class="charts-wrapper">

    <div class="chart-card">
      <div class="chart-card-label">Tensão</div>
      <div class="chart-live-value cv-color" id="cv">-- V</div>
      <canvas id="canvasV"></canvas>
    </div>

    <div class="chart-card">
      <div class="chart-card-label">Corrente</div>
      <div class="chart-live-value ci-color" id="ci">-- A</div>
      <canvas id="canvasI"></canvas>
    </div>

    <div class="chart-card">
      <div class="chart-card-label">Potência</div>
      <div class="chart-live-value cp-color" id="cp">-- W</div>
      <canvas id="canvasP"></canvas>
    </div>

  </div>
</div>

<script>
// ── Histórico de dados ─────────────────────────────────────
var MAX = 40;
var dadosV = [], dadosI = [], dadosP = [];

// ── Redimensiona canvas para resolução real ───────────────
function ajustarCanvas(canvas) {
  var rect = canvas.getBoundingClientRect();
  canvas.width  = rect.width  * window.devicePixelRatio;
  canvas.height = rect.height * window.devicePixelRatio;
}

// ── Desenha gráfico de linha no canvas ───────────────────
function desenharGrafico(canvasId, dados, cor) {
  var canvas = document.getElementById(canvasId);
  ajustarCanvas(canvas);
  var ctx = canvas.getContext('2d');
  var W = canvas.width, H = canvas.height;

  ctx.clearRect(0, 0, W, H);

  if (dados.length < 2) return;

  var min = Math.min.apply(null, dados);
  var max = Math.max.apply(null, dados);
  var span = (max - min) || 1;

  // Padding interno
  var padX = 4, padY = 10;
  var gW = W - padX * 2;
  var gH = H - padY * 2;

  // Área preenchida
  var grad = ctx.createLinearGradient(0, padY, 0, padY + gH);
  grad.addColorStop(0, cor + '55');
  grad.addColorStop(1, cor + '05');

  ctx.beginPath();
  for (var i = 0; i < dados.length; i++) {
    var x = padX + (i / (dados.length - 1)) * gW;
    var y = padY + gH - ((dados[i] - min) / span) * gH;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  // Fecha área
  ctx.lineTo(padX + gW, padY + gH);
  ctx.lineTo(padX, padY + gH);
  ctx.closePath();
  ctx.fillStyle = grad;
  ctx.fill();

  // Linha principal
  ctx.beginPath();
  for (var i = 0; i < dados.length; i++) {
    var x = padX + (i / (dados.length - 1)) * gW;
    var y = padY + gH - ((dados[i] - min) / span) * gH;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.strokeStyle = cor;
  ctx.lineWidth = 2.5 * window.devicePixelRatio;
  ctx.lineJoin = 'round';
  ctx.stroke();

  // Ponto atual (último)
  var xi = padX + gW;
  var yi = padY + gH - ((dados[dados.length-1] - min) / span) * gH;
  ctx.beginPath();
  ctx.arc(xi, yi, 4 * window.devicePixelRatio, 0, Math.PI * 2);
  ctx.fillStyle = cor;
  ctx.fill();
}

// ── Atualiza todos os gráficos ────────────────────────────
function atualizarGraficos() {
  desenharGrafico('canvasV', dadosV, '#38bdf8');
  desenharGrafico('canvasI', dadosI, '#fb923c');
  desenharGrafico('canvasP', dadosP, '#22c55e');
}

// ── Fetch dos dados do ESP32 ──────────────────────────────
function buscarDados() {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/dados', true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4 && xhr.status === 200) {
      try {
        var d = JSON.parse(xhr.responseText);

        // Tela principal
        document.getElementById('v').innerText = d.v.toFixed(2) + ' V';
        document.getElementById('i').innerText = d.i.toFixed(3) + ' A';
        document.getElementById('p').innerText = d.p.toFixed(2) + ' W';

        // Tela de gráficos
        document.getElementById('cv').innerText = d.v.toFixed(2) + ' V';
        document.getElementById('ci').innerText = d.i.toFixed(3) + ' A';
        document.getElementById('cp').innerText = d.p.toFixed(2) + ' W';

        // Histórico
        dadosV.push(d.v);
        dadosI.push(d.i);
        dadosP.push(d.p);
        if (dadosV.length > MAX) { dadosV.shift(); dadosI.shift(); dadosP.shift(); }

        atualizarGraficos();
      } catch(e) {}
    }
  };
  xhr.send();
}

// ── Navegação ─────────────────────────────────────────────
function irParaGraficos() {
  document.getElementById('main-screen').style.display  = 'none';
  document.getElementById('chart-screen').style.display = 'flex';
  // Redesenha para pegar dimensão correta após exibir
  setTimeout(atualizarGraficos, 50);
}

function voltarParaMain() {
  document.getElementById('chart-screen').style.display = 'none';
  document.getElementById('main-screen').style.display  = 'flex';
}

// ── Inicia polling ────────────────────────────────────────
buscarDados();
setInterval(buscarDados, 1000);
</script>
</body>
</html>
)rawliteral";