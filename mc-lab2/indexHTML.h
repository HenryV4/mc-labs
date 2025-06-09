#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char htmlPage[] PROGMEM = R"rawliteral(
<html>
  <head>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
        background-color: #f4f4f4;
        padding: 20px;
      }
      h1 {
        color: #333;
      }
      .button-container {
        margin: 20px auto;
      }
      button {
        background-color: #008CBA;
        border: none;
        color: white;
        padding: 15px 30px;
        text-align: center;
        display: inline-block;
        font-size: 16px;
        margin: 10px;
        cursor: pointer;
        border-radius: 5px;
        transition: 0.3s;
      }
      button:hover {
        background-color: #005f75;
      }
    </style>
  </head>
  <body>
    <h1>ESP8266 LED Control</h1>
    <div class="button-container">
      <button ondblclick="toggleOwnAlgorithm()">Toggle Own Algorithm</button>
      <button onclick="togglePartnerAlgorithm()">Toggle Partner Algorithm</button>
    </div>
    <script>
      function toggleOwnAlgorithm() {
        fetch('/toggleOwn').then(response => response.text()).then(data => console.log(data));
      }
      function togglePartnerAlgorithm() {
        fetch('/togglePartner').then(response => response.text()).then(data => console.log(data));
      }
    </script>
  </body>
</html>
)rawliteral";

#endif
