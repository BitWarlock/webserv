#!/usr/bin/env node

const fs = require('fs');
const os = require('os');
const crypto = require('crypto');

// Sample JSON data for processing
const sampleData = {
    users: [
        { id: 1, name: "Alice Johnson", email: "alice@example.com", role: "admin", lastLogin: "2024-01-15T10:30:00Z" },
        { id: 2, name: "Bob Smith", email: "bob@example.com", role: "user", lastLogin: "2024-01-14T15:45:00Z" },
        { id: 3, name: "Carol Davis", email: "carol@example.com", role: "moderator", lastLogin: "2024-01-13T09:20:00Z" },
        { id: 4, name: "David Wilson", email: "david@example.com", role: "user", lastLogin: "2024-01-12T14:10:00Z" },
        { id: 5, name: "Eve Brown", email: "eve@example.com", role: "admin", lastLogin: "2024-01-11T11:55:00Z" }
    ],
    products: [
        { id: "P001", name: "Laptop", price: 999.99, category: "Electronics", stock: 15 },
        { id: "P002", name: "Mouse", price: 29.99, category: "Electronics", stock: 50 },
        { id: "P003", name: "Keyboard", price: 79.99, category: "Electronics", stock: 25 },
        { id: "P004", name: "Monitor", price: 299.99, category: "Electronics", stock: 8 },
        { id: "P005", name: "Headphones", price: 149.99, category: "Audio", stock: 30 }
    ],
    orders: [
        { id: "O001", userId: 1, products: ["P001", "P002"], total: 1029.98, status: "completed" },
        { id: "O002", userId: 2, products: ["P003"], total: 79.99, status: "pending" },
        { id: "O003", userId: 3, products: ["P004", "P005"], total: 449.98, status: "shipped" }
    ]
};

function analyzeData(data) {
    const analysis = {
        userStats: {
            total: data.users.length,
            byRole: {},
            averageId: data.users.reduce((sum, user) => sum + user.id, 0) / data.users.length
        },
        productStats: {
            total: data.products.length,
            totalValue: data.products.reduce((sum, product) => sum + (product.price * product.stock), 0),
            categories: [...new Set(data.products.map(p => p.category))],
            averagePrice: data.products.reduce((sum, product) => sum + product.price, 0) / data.products.length
        },
        orderStats: {
            total: data.orders.length,
            totalRevenue: data.orders.reduce((sum, order) => sum + order.total, 0),
            byStatus: {}
        }
    };

    // Analyze users by role
    data.users.forEach(user => {
        analysis.userStats.byRole[user.role] = (analysis.userStats.byRole[user.role] || 0) + 1;
    });

    // Analyze orders by status
    data.orders.forEach(order => {
        analysis.orderStats.byStatus[order.status] = (analysis.orderStats.byStatus[order.status] || 0) + 1;
    });

    return analysis;
}

function getSystemInfo() {
    return {
        platform: os.platform(),
        arch: os.arch(),
        nodeVersion: process.version,
        memory: {
            total: os.totalmem(),
            free: os.freemem(),
            used: os.totalmem() - os.freemem()
        },
        cpus: os.cpus().length,
        uptime: os.uptime()
    };
}

function formatBytes(bytes) {
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    if (bytes === 0) return '0 B';
    const i = Math.floor(Math.log(bytes) / Math.log(1024));
    return Math.round(bytes / Math.pow(1024, i) * 100) / 100 + ' ' + sizes[i];
}

function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    return `${days}d ${hours}h ${minutes}m`;
}

// Process the data
const analysis = analyzeData(sampleData);
const systemInfo = getSystemInfo();

// Generate HTML output
console.log("Content-Type: text/html");
console.log("");
console.log("<!DOCTYPE html>");
console.log("<html>");
console.log("<head>");
console.log("<title>JSON Processor - JavaScript CGI</title>");
console.log("<style>");
console.log("body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }");
console.log(".container { max-width: 1200px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }");
console.log("h1 { color: #ffd700; text-align: center; margin-bottom: 30px; }");
console.log(".section { margin-bottom: 30px; }");
console.log(".section h2 { color: #ffd700; margin-bottom: 15px; }");
console.log(".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }");
console.log(".card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #ffd700; }");
console.log(".stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; margin-top: 15px; }");
console.log(".stat-item { text-align: center; padding: 10px; background: rgba(255,255,255,0.05); border-radius: 8px; }");
console.log(".stat-number { font-size: 1.5em; color: #ffd700; font-weight: bold; }");
console.log(".stat-label { color: #cccccc; font-size: 0.9em; margin-top: 5px; }");
console.log(".table { width: 100%; border-collapse: collapse; margin-top: 15px; }");
console.log(".table th, .table td { padding: 10px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); }");
console.log(".table th { background: rgba(255, 215, 0, 0.2); color: #ffd700; font-weight: bold; }");
console.log(".table tr:hover { background: rgba(255,255,255,0.05); }");
console.log(".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }");
console.log("</style>");
console.log("</head>");
console.log("<body>");
console.log("<div class='container'>");
console.log("<h1>JSON Data Processor</h1>");

// Data Analysis Section
console.log("<div class='section'>");
console.log("<h2>Data Analysis Results</h2>");
console.log("<div class='grid'>");

// User Statistics
console.log("<div class='card'>");
console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>User Statistics</h3>");
console.log(`<div class='stat-item'><div class='stat-number'>${analysis.userStats.total}</div><div class='stat-label'>Total Users</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${analysis.userStats.averageId.toFixed(1)}</div><div class='stat-label'>Average ID</div></div>`);
console.log("<div style='margin-top: 15px;'>");
console.log("<strong style='color: #ffd700;'>Users by Role:</strong>");
for (const [role, count] of Object.entries(analysis.userStats.byRole)) {
    console.log(`<div style='margin: 5px 0; color: #cccccc;'>${role}: ${count}</div>`);
}
console.log("</div>");
console.log("</div>");

// Product Statistics
console.log("<div class='card'>");
console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Product Statistics</h3>");
console.log(`<div class='stat-item'><div class='stat-number'>${analysis.productStats.total}</div><div class='stat-label'>Total Products</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.productStats.totalValue.toFixed(2)}</div><div class='stat-label'>Total Inventory Value</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.productStats.averagePrice.toFixed(2)}</div><div class='stat-label'>Average Price</div></div>`);
console.log("<div style='margin-top: 15px;'>");
console.log("<strong style='color: #ffd700;'>Categories:</strong>");
analysis.productStats.categories.forEach(category => {
    console.log(`<div style='margin: 5px 0; color: #cccccc;'>• ${category}</div>`);
});
console.log("</div>");
console.log("</div>");

// Order Statistics
console.log("<div class='card'>");
console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Order Statistics</h3>");
console.log(`<div class='stat-item'><div class='stat-number'>${analysis.orderStats.total}</div><div class='stat-label'>Total Orders</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>$${analysis.orderStats.totalRevenue.toFixed(2)}</div><div class='stat-label'>Total Revenue</div></div>`);
console.log("<div style='margin-top: 15px;'>");
console.log("<strong style='color: #ffd700;'>Orders by Status:</strong>");
for (const [status, count] of Object.entries(analysis.orderStats.byStatus)) {
    console.log(`<div style='margin: 5px 0; color: #cccccc;'>${status}: ${count}</div>`);
}
console.log("</div>");
console.log("</div>");

console.log("</div>");
console.log("</div>");

// System Information Section
console.log("<div class='section'>");
console.log("<h2>System Information</h2>");
console.log("<div class='card'>");
console.log("<div class='stat-grid'>");
console.log(`<div class='stat-item'><div class='stat-number'>${systemInfo.platform}</div><div class='stat-label'>Platform</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${systemInfo.arch}</div><div class='stat-label'>Architecture</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${systemInfo.nodeVersion}</div><div class='stat-label'>Node.js Version</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${systemInfo.cpus}</div><div class='stat-label'>CPU Cores</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${formatBytes(systemInfo.memory.total)}</div><div class='stat-label'>Total Memory</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${formatBytes(systemInfo.memory.free)}</div><div class='stat-label'>Free Memory</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${formatUptime(systemInfo.uptime)}</div><div class='stat-label'>System Uptime</div></div>`);
console.log(`<div class='stat-item'><div class='stat-number'>${Math.round((systemInfo.memory.used / systemInfo.memory.total) * 100)}%</div><div class='stat-label'>Memory Usage</div></div>`);
console.log("</div>");
console.log("</div>");
console.log("</div>");

// Sample Data Table
console.log("<div class='section'>");
console.log("<h2>Sample Data Preview</h2>");
console.log("<div class='card'>");
console.log("<h3 style='color: #ffd700; margin-bottom: 15px;'>Users</h3>");
console.log("<table class='table'>");
console.log("<thead><tr><th>ID</th><th>Name</th><th>Email</th><th>Role</th><th>Last Login</th></tr></thead>");
console.log("<tbody>");
sampleData.users.forEach(user => {
    console.log(`<tr><td>${user.id}</td><td>${user.name}</td><td>${user.email}</td><td>${user.role}</td><td>${user.lastLogin}</td></tr>`);
});
console.log("</tbody></table>");
console.log("</div>");
console.log("</div>");

console.log("<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>");
console.log("</div>");
console.log("</body>");
console.log("</html>"); 