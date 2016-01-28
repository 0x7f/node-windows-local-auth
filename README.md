# node-windows-local-auth
Authentication for node.js using local windows user accounts.

## Example

```
var windowsLocalAuth = require('windows-local-auth');

windowsLocalAuth(someUser, somePassword, function(err, success) {
    if (err) { throw err; }

    // handle success
});
```
