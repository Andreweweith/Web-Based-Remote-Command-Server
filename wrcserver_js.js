var stdout;
var stderr;

window.onload = function() {
    var formRun = document.getElementById("run");
    var formCommand = document.getElementById("command");

    var commandBox = document.getElementById("combox");

    stdout = document.getElementById("stdout");
    stderr = document.getElementById("stderr");

    formRun.onclick = function run() {
        var commandText = document.getElementById("command").value;

        formCommand.value = "";

        commandBox.value = "";
        commandBox.value += commandText;

        stdout.value = "";
        stdout.value += "Stdout";
        stderr.value = "";
        stderr.value += "Stderr";

        var request = new XMLHttpRequest();
        request.open("POST","/exec?cmd="+commandText,true);
        request.onload = handleResponse;

        request.send(null);
    }
}




function handleResponse() {
    if (this.status == 200 && this.responseText != null) {

        document.getElementById("combox").textContent =
                                    document.getElementById("command").textContent;

        var response = JSON.parse(this.responseText);

        stdout.value += response.stdout;
        stderr.value += response.stderr;
    }
    else if (this.status == 404) {
        console.log("404 - Error has occurred");
    }
}
