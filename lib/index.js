var bindings = require('./bindings');

var IS_SUPPORTED_PLATFORM = /^win/.test(process.platform);

function checkUserPassword(user, password, domain, cb) {
    if (typeof domain === 'function') {
        cb = domain;
        domain = '';
    }

    if (user.indexOf('@') !== -1) {
        domain = user.substr(user.indexOf('@') + 1)
        user = user.substr(0, user.indexOf('@'))
    } else if (user.indexOf('\\') {
        domain = user.substr(0, user.indexOf('\\'))
        user = user.substr(user.indexOf('\\') + 1)
    }

    if (typeof cb !== 'function') {
        throw new TypeError("callback must be a function");
    }
    return bindings.checkUserPassword(domain, user, password, cb);
};

module.exports = checkUserPassword;
module.exports.IS_SUPPORTED_PLATFORM = IS_SUPPORTED_PLATFORM;
