console.log("MultithreadedServer - JavaScript loaded!");

function testEcho() {
    const input = document.getElementById('echoInput');
    const result = document.getElementById('echoResult');
    const text = input.value.trim();
    
    if (!text) {
        alert('Please enter some text to echo');
        return;
    }
    
    // Show loading
    result.style.display = 'block';
    result.innerHTML = 'Sending to server...';
    
    fetch('/echo', {
        method: 'POST',
        headers: {
            'Content-Type': 'text/plain'
        },
        body: text
    })
    .then(response => response.text())
    .then(data => {
        result.innerHTML = '<strong>Server Response:</strong><br>' + data.replace(/\n/g, '<br>');
    })
    .catch(error => {
        result.innerHTML = '<strong>Error:</strong> ' + error;
    });
}
