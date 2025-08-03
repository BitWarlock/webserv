<?php
// Simulate database operations and session management
session_start();

// Simulate database
$users_db = [
    ['id' => 1, 'username' => 'admin', 'email' => 'admin@webserv.com', 'role' => 'admin', 'last_login' => '2024-01-15 10:30:00', 'status' => 'active'],
    ['id' => 2, 'username' => 'user1', 'email' => 'user1@webserv.com', 'role' => 'user', 'last_login' => '2024-01-14 15:45:00', 'status' => 'active'],
    ['id' => 3, 'username' => 'moderator', 'email' => 'mod@webserv.com', 'role' => 'moderator', 'last_login' => '2024-01-13 09:20:00', 'status' => 'active'],
    ['id' => 4, 'username' => 'guest', 'email' => 'guest@webserv.com', 'role' => 'guest', 'last_login' => '2024-01-12 14:10:00', 'status' => 'inactive'],
    ['id' => 5, 'username' => 'tester', 'email' => 'test@webserv.com', 'role' => 'user', 'last_login' => '2024-01-11 11:55:00', 'status' => 'active']
];

$sessions_db = [
    ['session_id' => 'sess_001', 'user_id' => 1, 'ip_address' => '192.168.1.100', 'created' => '2024-01-15 10:30:00', 'expires' => '2024-01-15 18:30:00'],
    ['session_id' => 'sess_002', 'user_id' => 2, 'ip_address' => '192.168.1.101', 'created' => '2024-01-14 15:45:00', 'expires' => '2024-01-14 23:45:00'],
    ['session_id' => 'sess_003', 'user_id' => 3, 'ip_address' => '192.168.1.102', 'created' => '2024-01-13 09:20:00', 'expires' => '2024-01-13 17:20:00']
];

$logs_db = [
    ['id' => 1, 'user_id' => 1, 'action' => 'login', 'timestamp' => '2024-01-15 10:30:00', 'ip' => '192.168.1.100'],
    ['id' => 2, 'user_id' => 2, 'action' => 'upload_file', 'timestamp' => '2024-01-14 15:45:00', 'ip' => '192.168.1.101'],
    ['id' => 3, 'user_id' => 1, 'action' => 'delete_file', 'timestamp' => '2024-01-14 16:20:00', 'ip' => '192.168.1.100'],
    ['id' => 4, 'user_id' => 3, 'action' => 'create_folder', 'timestamp' => '2024-01-13 09:20:00', 'ip' => '192.168.1.102'],
    ['id' => 5, 'user_id' => 2, 'action' => 'download_file', 'timestamp' => '2024-01-14 17:30:00', 'ip' => '192.168.1.101']
];

// Get current session info
$current_session = [
    'session_id' => session_id(),
    'user_id' => $_SESSION['user_id'] ?? null,
    'ip_address' => $_SERVER['REMOTE_ADDR'] ?? 'Unknown',
    'created' => date('Y-m-d H:i:s'),
    'expires' => date('Y-m-d H:i:s', strtotime('+8 hours'))
];

// Simulate session creation
if (!isset($_SESSION['user_id'])) {
    $_SESSION['user_id'] = rand(1, 5);
    $_SESSION['created'] = time();
}

// Get user info
$current_user = null;
foreach ($users_db as $user) {
    if ($user['id'] == $_SESSION['user_id']) {
        $current_user = $user;
        break;
    }
}

// Calculate statistics
$total_users = count($users_db);
$active_users = count(array_filter($users_db, function($user) { return $user['status'] == 'active'; }));
$total_sessions = count($sessions_db);
$total_logs = count($logs_db);

$role_stats = [];
foreach ($users_db as $user) {
    $role = $user['role'];
    $role_stats[$role] = ($role_stats[$role] ?? 0) + 1;
}

$action_stats = [];
foreach ($logs_db as $log) {
    $action = $log['action'];
    $action_stats[$action] = ($action_stats[$action] ?? 0) + 1;
}

// Output HTML
header('Content-Type: text/html');
?>
<!DOCTYPE html>
<html>
<head>
    <title>Session Manager - PHP CGI</title>
    <style>
        body { 
            font-family: 'Segoe UI', sans-serif; 
            background: #1a1a1a; 
            color: #ffffff; 
            margin: 20px; 
        }
        .container { 
            max-width: 1200px; 
            margin: 0 auto; 
            background: rgba(255,255,255,0.05); 
            padding: 30px; 
            border-radius: 15px; 
        }
        h1 { 
            color: #607d8b; 
            text-align: center; 
            margin-bottom: 30px; 
        }
        .section { 
            margin-bottom: 30px; 
        }
        .section h2 { 
            color: #607d8b; 
            margin-bottom: 15px; 
        }
        .grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); 
            gap: 20px; 
        }
        .card { 
            background: rgba(255,255,255,0.1); 
            padding: 20px; 
            border-radius: 10px; 
            border-left: 4px solid #607d8b; 
        }
        .stat-grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); 
            gap: 15px; 
            margin-top: 15px; 
        }
        .stat-item { 
            text-align: center; 
            padding: 10px; 
            background: rgba(255,255,255,0.05); 
            border-radius: 8px; 
        }
        .stat-number { 
            font-size: 1.5em; 
            color: #607d8b; 
            font-weight: bold; 
        }
        .stat-label { 
            color: #cccccc; 
            font-size: 0.9em; 
            margin-top: 5px; 
        }
        .table { 
            width: 100%; 
            border-collapse: collapse; 
            margin-top: 15px; 
        }
        .table th, .table td { 
            padding: 10px; 
            text-align: left; 
            border-bottom: 1px solid rgba(255,255,255,0.1); 
        }
        .table th { 
            background: rgba(96, 125, 139, 0.2); 
            color: #607d8b; 
            font-weight: bold; 
        }
        .table tr:hover { 
            background: rgba(255,255,255,0.05); 
        }
        .status-active { 
            color: #4caf50; 
            font-weight: bold; 
        }
        .status-inactive { 
            color: #f44336; 
            font-weight: bold; 
        }
        .back-btn { 
            background: #666; 
            color: white; 
            padding: 10px 20px; 
            text-decoration: none; 
            border-radius: 5px; 
            display: inline-block; 
            margin-top: 20px; 
        }
        .current-session { 
            background: rgba(76, 175, 80, 0.1); 
            border-left-color: #4caf50; 
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Session Manager</h1>
        
        <!-- Current Session -->
        <div class="section">
            <h2>Current Session</h2>
            <div class="card current-session">
                <div class="stat-grid">
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $current_session['session_id']; ?></div>
                        <div class="stat-label">Session ID</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $current_user['username'] ?? 'Unknown'; ?></div>
                        <div class="stat-label">Current User</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $current_user['role'] ?? 'guest'; ?></div>
                        <div class="stat-label">User Role</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $current_session['ip_address']; ?></div>
                        <div class="stat-label">IP Address</div>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- Database Statistics -->
        <div class="section">
            <h2>Database Statistics</h2>
            <div class="grid">
                <div class="card">
                    <h3 style="color: #607d8b; margin-bottom: 15px;">User Statistics</h3>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $total_users; ?></div>
                        <div class="stat-label">Total Users</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $active_users; ?></div>
                        <div class="stat-label">Active Users</div>
                    </div>
                    <div style="margin-top: 15px;">
                        <strong style="color: #607d8b;">Users by Role:</strong>
                        <?php foreach ($role_stats as $role => $count): ?>
                            <div style="margin: 5px 0; color: #cccccc;"><?php echo ucfirst($role); ?>: <?php echo $count; ?></div>
                        <?php endforeach; ?>
                    </div>
                </div>
                
                <div class="card">
                    <h3 style="color: #607d8b; margin-bottom: 15px;">Session Statistics</h3>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $total_sessions; ?></div>
                        <div class="stat-label">Active Sessions</div>
                    </div>
                    <div class="stat-item">
                        <div class="stat-number"><?php echo $total_logs; ?></div>
                        <div class="stat-label">Total Log Entries</div>
                    </div>
                    <div style="margin-top: 15px;">
                        <strong style="color: #607d8b;">Recent Actions:</strong>
                        <?php foreach ($action_stats as $action => $count): ?>
                            <div style="margin: 5px 0; color: #cccccc;"><?php echo ucfirst(str_replace('_', ' ', $action)); ?>: <?php echo $count; ?></div>
                        <?php endforeach; ?>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- Users Table -->
        <div class="section">
            <h2>User Database</h2>
            <div class="card">
                <table class="table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Username</th>
                            <th>Email</th>
                            <th>Role</th>
                            <th>Last Login</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($users_db as $user): ?>
                            <tr>
                                <td><?php echo $user['id']; ?></td>
                                <td><?php echo $user['username']; ?></td>
                                <td><?php echo $user['email']; ?></td>
                                <td><?php echo ucfirst($user['role']); ?></td>
                                <td><?php echo $user['last_login']; ?></td>
                                <td class="<?php echo $user['status'] == 'active' ? 'status-active' : 'status-inactive'; ?>">
                                    <?php echo ucfirst($user['status']); ?>
                                </td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            </div>
        </div>
        
        <!-- Activity Logs -->
        <div class="section">
            <h2>Recent Activity Logs</h2>
            <div class="card">
                <table class="table">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>User ID</th>
                            <th>Action</th>
                            <th>Timestamp</th>
                            <th>IP Address</th>
                        </tr>
                    </thead>
                    <tbody>
                        <?php foreach ($logs_db as $log): ?>
                            <tr>
                                <td><?php echo $log['id']; ?></td>
                                <td><?php echo $log['user_id']; ?></td>
                                <td><?php echo ucfirst(str_replace('_', ' ', $log['action'])); ?></td>
                                <td><?php echo $log['timestamp']; ?></td>
                                <td><?php echo $log['ip']; ?></td>
                            </tr>
                        <?php endforeach; ?>
                    </tbody>
                </table>
            </div>
        </div>
        
        <a href="/cgi-bin/cgi_test.html" class="back-btn">Back to CGI Testing</a>
    </div>
</body>
</html> 