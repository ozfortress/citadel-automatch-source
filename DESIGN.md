# Design Document

This document contains designs for the plugin.

## Simple Runthrough:

A player types `!match` into chat.
Plugin queries all endpoints to see if a match exists which has approximately the same players playing as are in the server.
Plugin: `Found <league> match: <home team> vs <away team> (<web link>).`
Plugin queries league to check if teams are currently valid (including mercs).
Plugin records server state and listens for rcon commands.
Plugin: `Asking one player from both teams to confirm. Please type !confirm or !cancel.`
Plugin opens MOTD for the person who typed `!match` onto the confirm page for the match.
Some player types `!confirm` into chat. Plugin opens MOTD for them onto the confirm page.
Plugin continually queries league, waiting for one player from each team to confirm or a timeout.
Once confirmed, Plugin: `Match: <home team> vs <away team> is now active`
<!-- Plugin loads the correct map for the match, if the server isn't on that map already. -->
Plugin sets team names to those of the teams.
Plugin loads and executes match config for league.
Plugin waits until teams ready up.
Plugin: `Match: <home team> vs <away team> is not live! Good luck`
Plugin records when points are scores by the team: `<team> scored a point. <winning team> is currently leading <score> to <score>.` (etc.))
Plugin records and monitors for connecting players, dropping those that aren't supposed to be in the match except for admins.
Plugin records pauses and other game events.
<!-- TODO: Subs/Mercs -->
<!-- TODO: Golden Cap -->
Plugin waits until map is finished.
<!-- TODO: Extra maps -->
Plugin submits scores to league, including logs.
