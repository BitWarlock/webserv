<?php
header("Content-Type: text/html");
?>
<html>
<head><title>PHP CGI Test</title></head>
<body>
<h1>PHP CGI Test</h1>
<p>Request Method: <?= $_SERVER['REQUEST_METHOD'] ?? 'N/A' ?></p>
<p>Query String: <?= $_SERVER['QUERY_STRING'] ?? 'N/A' ?></p>
<p>Script Name: <?= $_SERVER['SCRIPT_NAME'] ?? 'N/A' ?></p>
<p>Content Length: <?= $_SERVER['CONTENT_LENGTH'] ?? 'N/A' ?></p>
<p>Current Working Directory: <?= getcwd() ?></p>
<p>Script Path: <?= __FILE__ ?></p>
<p>PHP Path: <?= PHP_BINARY ?></p>
</body>
</html>
