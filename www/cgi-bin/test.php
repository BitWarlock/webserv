<?php
echo "Content-Type: text/html\r\n";
echo "\r\n";
echo "<html>";
echo "<head><title>PHP CGI Test</title></head>";
echo "<body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>Request Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Query String: " . $_SERVER['QUERY_STRING'] . "</p>";
echo "<p>Script Name: " . $_SERVER['SCRIPT_NAME'] . "</p>";
echo "<p>Content Length: " . $_SERVER['CONTENT_LENGTH'] . "</p>";
echo "</body>";
echo "</html>";
?>
