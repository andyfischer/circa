function onLoad(event) {
    nacl_module = document.getElementById('nacl_module');
    nacl_module.addEventListener('message', handleMessage, false);

    console.log("onLoad");
    var replForm = document.getElementById('replForm');
    replForm.addEventListener('submit', function(event) {
        event.preventDefault();
        sendCommand(document.getElementById('textInput').value);
    }, false);
}

function printLine(line) {
    repl_output.appendChild(document.createTextNode(line));
    repl_output.appendChild(document.createElement('br'));
}

function handleMessage(event) {
    var msg = JSON.parse(event.data);

    var repl_output = document.getElementById('repl_output');

    for (var i in msg) {
        printLine(msg[i]);
    }
}

function sendCommand(cmd) {
    console.log('sending: ' + cmd);
    printLine('> ' + cmd);
    nacl_module.postMessage(cmd);
}

window.addEventListener('load', onLoad);
