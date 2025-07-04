const test = require('node:test');
const assert = require('node:assert');   
const windowsLocalAuth = require('../lib');

test('is running on supported platform', function() {
    assert(windowsLocalAuth.IS_SUPPORTED_PLATFORM);
});

test('is function available', function() {
    assert(typeof windowsLocalAuth === 'function');
});

test('fails with invalid username', function(t, done) {
    windowsLocalAuth('invaliduser', 'invalidpassword', function(err, success, isAdmin) {
        assert(success === false);
        done(); 
    });
})
