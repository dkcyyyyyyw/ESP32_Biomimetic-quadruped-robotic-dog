static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🤖 机械狗控制器</title>
    <style>
        /* 基础样式 */
        body { background: #f0f2f5; font-family: Arial; text-align: center; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 15px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; margin-bottom: 30px; }
        
        /* 按钮样式 */
        .control-group { display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px; margin-bottom: 30px; }
        button { padding: 15px 25px; border: none; border-radius: 8px; cursor: pointer; transition: 0.3s; font-size: 16px; }
        
        .movement { background: #3498db; color: white; }
        .action { background: #2ecc71; color: white; }
        .buzzer { background: #e74c3c; color: white; }
        .led { background: #9b59b6; color: white; }
        
        button:hover { opacity: 0.9; transform: translateY(-2px); }
        
        /* 增强样式 */
        button:active, .active-button {
            background: #3a5f5f !important;
            box-shadow: inset 0 2px 4px rgba(0,0,0,0.2);
        }
        
        #pauseButton.paused {
            background: #ff4444 !important;
        }
        
        .disabled {
            opacity: 0.5;
            pointer-events: none;
        }
        
        .led-button {
            height: 50px;
            width: 50px;
            border-radius: 50%;
            margin: 0 10px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        }
        
        .led-button.red { background-color: #e74c3c; }
        .led-button.green { background-color: #2ecc71; }
        .led-button.blue { background-color: #3498db; }
        
        .led-group {
            display: flex;
            justify-content: center;
            margin: 20px 0;
        }
        
        .status { margin-top: 20px; padding: 15px; background: #ecf0f1; border-radius: 8px; color: #2c3e50; }
    </style>
</head>
<body>
<div class="container">
    <h1>🤖 机械狗控制器</h1>
    
    <div class="control-group">
        <div></div>
        <button class="movement" 
                onmousedown="startAction('move',1)" 
                ontouchstart="startAction('move',1)"
                onmouseup="stopAction('move',3)" 
                ontouchend="stopAction('move',3)">↑ 前进</button>
        <div></div>
        
        <button class="movement" 
                onmousedown="startAction('move',2)" 
                ontouchstart="startAction('move',2)"
                onmouseup="stopAction('move',6)" 
                ontouchend="stopAction('move',6)">← 左转</button>
                
        <button class="movement" 
                onmousedown="startAction('move',5)" 
                ontouchstart="startAction('move',5)"
                onmouseup="stopAction('move',3)" 
                ontouchend="stopAction('move',3)">↓ 后退</button>
                
        <button class="movement" 
                onmousedown="startAction('move',4)" 
                ontouchstart="startAction('move',4)"
                onmouseup="stopAction('move',6)" 
                ontouchend="stopAction('move',6)">→ 右转</button>
        
        <button class="action" onclick="sendCommand('funcMode',2)">🪑 蹲下</button>
        <button class="action" onclick="sendCommand('funcMode',3)">🤝 握手</button>
        <button class="action" onclick="sendCommand('funcMode',4)">🦘 跳跃</button>
        
        <div></div>
        <button id="pauseButton" class="action" onclick="togglePause()">🛑 静止</button>
        <div></div>
    </div>

    <!-- 改为红绿蓝三个颜色按钮 -->
    <div class="led-group">
        <button class="led-button red" onclick="setLEDColor(0, 255, 0)">红灯</button>
        <button class="led-button green" onclick="setLEDColor(255, 0, 0)">绿灯</button>
        <button class="led-button blue" onclick="setLEDColor(0, 0, 255)">蓝灯</button>
    </div>

    <div class="control-group">
        <button class="buzzer" onclick="sendCommand('buzzer',1)">🔊 蜂鸣器开</button>
        <button class="buzzer" onclick="sendCommand('buzzer',0)">🔇 蜂鸣器关</button>
        <button class="action" onclick="sendCommand('funcMode',1)">🧍 直立</button>
    </div>

    <div class="status" id="status">状态: 就绪</div>
    <div class="status" id="lastAction">最后操作: 无</div>
</div>

<script>
    let actionInterval = null;
    let currentAction = null;
    let isPaused = false;

    // 长按触发持续动作
    function startAction(variable, value) {
        if (isPaused) return;
        event.target.classList.add('active-button');
        currentAction = { variable, value };
        sendCommand(variable, value);
        actionInterval = setInterval(() => {
            sendCommand(variable, value);
        }, 100);
    }

    // 松开停止动作
    function stopAction(variable, stopValue) {
        if (isPaused) return;
        event.target.classList.remove('active-button');
        if (actionInterval) {
            clearInterval(actionInterval);
            actionInterval = null;
            sendCommand(variable, stopValue);
        }
    }

    // 静止/恢复控制
    function togglePause() {
        isPaused = !isPaused;
        const pauseButton = document.getElementById('pauseButton');
        const controls = document.querySelectorAll('button:not(#pauseButton)');
        
        if (isPaused) {
            sendCommand('move', 3);
            sendCommand('move', 6);
            sendCommand('funcMode', 0);
            pauseButton.classList.add('paused');
            pauseButton.textContent = '▶️ 恢复';
            controls.forEach(control => control.classList.add('disabled'));
            updateStatus('已暂停');
            if (actionInterval) {
                clearInterval(actionInterval);
                actionInterval = null;
            }
        } else {
            pauseButton.classList.remove('paused');
            pauseButton.textContent = '🛑 静止';
            controls.forEach(control => control.classList.remove('disabled'));
            updateStatus('已恢复');
        }
    }

    // 命令发送 - 兼容两种接口
    function sendCommand(variable, value) {
        if (isPaused && variable !== 'move' && variable !== 'funcMode' && variable !== 'buzzer') return;
        
        // 兼容第一段代码的命令格式
        let cmdMap = {
            'move': {
                1: 'forward',
                2: 'left',
                3: 'stop_forward',
                4: 'right',
                5: 'backward',
                6: 'stop_turn'
            },
            'funcMode': {
                1: 'stand_up',
                2: 'sit',
                3: 'shake',
                4: 'jump',
                0: 'idle'
            },
            'buzzer': {
                1: 'beep_on',
                0: 'beep_off'
            },
            'ledr': 'set_led_r',
            'ledg': 'set_led_g',
            'ledb': 'set_led_b'
        };
        
        // 构建命令
        let cmd = '';
        if (variable === 'ledr' || variable === 'ledg' || variable === 'ledb') {
            cmd = `${cmdMap[variable]}/${value}`;
        } else {
            cmd = cmdMap[variable][value] || `${variable}/${value}`;
        }
        
        // 发送命令 - 同时支持两种URL格式
        var xhr = new XMLHttpRequest();
        xhr.open('GET', `/${cmd}`, true);
        xhr.send();
        
        // 也发送第二段代码的格式，提高兼容性
        var xhr2 = new XMLHttpRequest();
        xhr2.open('GET', `/control?var=${variable}&val=${value}&cmd=0`, true);
        xhr2.send();
        
        updateStatus(`正在执行 ${variable}=${value}`);
        updateLastAction(`${variable}=${value}`);
        
        // 非持续命令2秒后恢复状态显示
        if (!['move'].includes(variable)) {
            setTimeout(() => {
                updateStatus('状态: 就绪');
            }, 2000);
        }
    }

    // 设置LED颜色
    function setLEDColor(red, green, blue) {
        if (isPaused) return;
        
        // 发送RGB值
        sendCommand('ledr', red);
        sendCommand('ledg', green);
        sendCommand('ledb', blue);
        
        // 更新状态
        const colorNames = {
            '0,255,0': '红色',
            '255,0,0': '绿色',
            '0,0,255': '蓝色'
        };
        const colorName = colorNames[`${red},${green},${blue}`] || '自定义';
        updateStatus(`灯光已设置为 ${colorName}`);
    }

    // 更新状态显示
    function updateStatus(message) {
        document.getElementById("status").innerHTML = message;
    }

    // 更新最后操作记录
    function updateLastAction(message) {
        document.getElementById("lastAction").innerHTML = `最后操作: ${message} (${new Date().toLocaleTimeString()})`;
    }
</script>
</body>
</html>
    
)rawliteral";