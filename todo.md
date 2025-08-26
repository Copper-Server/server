[ ] fix memory session memory leak, the sessions not released

[ ] fix UB while adding commands in WorldManagementPlugin::OnCommandsLoad in `auto worlds = browser.add_child("worlds");` line (I have no idea why it breaks and why sometimes not)