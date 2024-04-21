const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html>
  <head>
    <title>Data Logger</title>
    <h1 style="text-align:center; color:red;">Attendance System</h1>
    <h3 style="text-align:center;">NodeMCU Data Logger</h3>

    <form METHOD="POST"action="/postForm">
    <input type="text" name="myText" value="">
    <input type="submit" value="submit">
    </form>
    <br></br>
    <style>
    
    canvas{
      -moz-user-select: none;
      -webkit-user-select: none;
      -ms-user-select: none;
    }
    /* Data Table Styling*/ 
    #dataTable {
      font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
      border-collapse: collapse;
      width: 100%;
      text-align: center;
    }
    #dataTable td, #dataTable th {
      border: 1px solid #ddd;
      padding: 8px;
    }
    #dataTable tr:nth-child(even){background-color: #f2f2f2;}
    #dataTable tr:hover {background-color: #ddd;}
    #dataTable th {
      padding-top: 12px;
      padding-bottom: 12px;
      text-align: center;
      background-color: #050505;
      color: white;
    }
    </style>
  </head>
  <body>
    <div>
      <table id="dataTable">
        <tr><th>Time</th><th>Id - Name</th><th>Status</th></tr>
      </table>
    </div>
  <br>
  <br>  
  <script>
    var Tvalues = [];
    var Hvalues = [];
    var timeStamp = [];
    setInterval(function() {
      // Call a function repetatively with 5 Second interval
      getData();
    }, 2000); //5000mSeconds update rate
     function getData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
         //Push the data in array
      var time = new Date().toLocaleTimeString();
      var txt = this.responseText;
      var obj = JSON.parse(txt); 
          Tvalues.push(obj.Id);
          Hvalues.push(obj.Status);
          timeStamp.push(time);
      //Update Data Table
        var table = document.getElementById("dataTable");
        var row = table.insertRow(1); //Add after headings
        var cell1 = row.insertCell(0);
        var cell2 = row.insertCell(1);
        var cell3 = row.insertCell(2);
        cell1.innerHTML = time;
        cell2.innerHTML = obj.Id;
        cell3.innerHTML = obj.Status;
        }
      };
      xhttp.open("GET", "readData", true); //Handle readData server on ESP8266
      xhttp.send();
    }
    </script>
  </body>
</html>

)=====";
 
