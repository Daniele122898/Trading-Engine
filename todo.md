# Rewrite
- [x] ME single threaded, blocked,
- [ ] Respond with action immediately
- [ ] Don't actually safe orders (unless order specifies it wants to be safed.)
- [ ] Drop Copy
- [ ] Ensure Deletion is propagated

# TODO
- [ ] Money / Economy / Position Limits
- [ ] Liquidity Bots
- [ ] Improve the response on order post (matching response instead of relying on ws)
- [ ] If 2 offers match, send only aggressor over info line?


# Done
- [x] dont broadcast creating for IOC / FOK etc. 
- [x] Ensure matching engine is actually single threaded using a queue
- [x] Partially filled orders change current quantity on startup
- [x] Make Symbol modification admin only
- [x] Allow for cancellation of orders
- [x] end of trading day
- [x] Expiry of orders
- [x] Ratelimit
- [x] WebSocket
- [x] broadcast creation of orders

# Postponed
- [ ] Potentially have 2 separate WS for private and info line?
- [ ] Allow for modifications of orders
- [ ] Allow for quotes
