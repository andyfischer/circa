function onLoad(event) {
    nacl_module = document.getElementById('nacl_module');
    nacl_module.addEventListener('message', handleMessage, false);

    console.log("onLoad");
    var replForm = document.getElementById('replForm');
    replForm.addEventListener('submit', function(event) {
        console.log('submit');
        event.preventDefault();
        sendCommand(document.getElementById('textInput').value);
    }, false);
}

function handleMessage(event) {
    var repl_output = document.getElementById('repl_output');
    repl_output.appendChild(document.createTextNode(event.data));
    repl_output.appendChild(document.createElement('br'));
}

function sendCommand(cmd) {
    console.log('sending: ' + cmd);
    nacl_module.postMessage(cmd);
}

window.addEventListener('load', onLoad);
