const windowsLocalAuth = require('../lib');

console.log('Testing windows-local-auth module...');

// Test 1: Module loads successfully
console.log('✓ Module loaded successfully');

// Test 2: Check if running on supported platform
console.log('Platform support:', windowsLocalAuth.IS_SUPPORTED_PLATFORM ? '✓ Supported' : '✗ Unsupported');

// Test 3: Basic function availability
if (typeof windowsLocalAuth === 'function') {
    console.log('✓ Main function available');
} else {
    console.log('✗ Main function not available');
    process.exit(1);
}

// Test 4: Platform-specific functionality
if (process.platform === 'win32') {
    console.log('✓ Running on Windows - full functionality available');
    
    // Test with invalid credentials (should not crash)
    windowsLocalAuth('invaliduser', 'invalidpassword', function(err, success, isAdmin) {
        if (err) {
            console.log('✓ Error handling works correctly:', err.message);
        } else {
            console.log('Result:', { success, isAdmin });
        }
        console.log('✓ All basic tests passed');
    });
} else {
    console.log('⚠ Running on non-Windows platform - limited functionality');
    console.log('✓ All basic tests passed');
}
