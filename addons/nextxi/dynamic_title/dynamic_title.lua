local account = require('account')
local windower = require('core.windower')

-- Listen for the login event from the account service
account.login:register(function()
    if account.logged_in then
        windower.set_window_title("Next XI - " .. account.name)
    end
end)

-- Listen for the logout event to reset it
account.logout:register(function()
    windower.set_window_title("Next XI")
end)