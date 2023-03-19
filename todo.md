# TODO
- [ ] Money / Economy / Position Limits
- [ ] Liquidity Bots
- [ ] Ensure matching engine is actually single threaded using a queue
- [ ] Improve the response on order post (matching response instead of relying on ws)
- [ ] Potentially have 2 separate WS for private and info line?
- [ ] dont broadcast creating for IOC / FOK etc. 
- [ ] If 2 offers match, send only aggressor over info line?


# Done
- [x] Partially filled orders change current quantity on startup
- [x] Make Symbol modification admin only
- [x] Allow for cancellation of orders
- [x] end of trading day
- [x] Expiry of orders
- [x] Ratelimit
- [x] WebSocket
- [x] broadcast creation of orders

# Postponed
- [ ] Allow for modifications of orders
- [ ] Allow for quotes
