var stdout;
var stderr;
// On window load, initialize variables with the HTML elements
window.onload = function() {
    var formRun = document.getElementById("run");
    var formCommand = document.getElementById("command");

    var commandBox = document.getElementById("combox");

    stdout = document.getElementById("stdout");
    stderr = document.getElementById("stderr");

    // When button is clicked, disable to prevent further input,
    // Then populate the commandBox with the input command,
    // open/handle then send request. Re-enable button when finished.
    formRun.onclick = function run() {
        formRun.disabled= true;
        var commandText = document.getElementById("command").value;

        formCommand.value = "";

        commandBox.value += commandText + "\n";

        var request = new XMLHttpRequest();
        request.open("POST","/exec?cmd="+commandText,true);
        request.onload = handleResponse;

        request.send(null);
        formRun.disabled= false;
    }
}

// Function to handle response, checks for correct status then process response text
function handleResponse() {
    if (this.status == 200 && this.responseText != null) {

        var response = JSON.parse(this.responseText);

        stdout.value += response.stdout + "\n";
        stderr.value += response.stderr + "\n";
    }
    else if (this.status == 404) {
        console.log("404 - Error has occurred");
    }
}
