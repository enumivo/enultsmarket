#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::buy(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  
  enu_balance = enu_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  enu_balance = enu_balance-received;  

  // get LTS balance
  double lts_balance = enumivo::token(N(ltsonenumivo)).
	   get_balance(_self, enumivo::symbol_type(LTS_SYMBOL).name()).amount;

  lts_balance = lts_balance/10000;

  double buy = 10000*(received*lts_balance)/(enu_balance+(2*received));

  auto to = transfer.from;

  auto quantity = asset(buy, LTS_SYMBOL);

  action(permission_level{_self, N(active)}, N(ltsonenumivo), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Buy LTS with ENU")))
      .send();
}

void ex::sell(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get LTS balance
  double lts_balance = enumivo::token(N(ltsonenumivo)).
	   get_balance(_self, enumivo::symbol_type(LTS_SYMBOL).name()).amount;
  
  lts_balance = lts_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  lts_balance = lts_balance-received;  

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  enu_balance = enu_balance/10000;

  double sell = 10000*(received*enu_balance)/(lts_balance+(2*received));

  auto to = transfer.from;

  auto quantity = asset(sell, ENU_SYMBOL);

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Sell LTS for ENU")))
      .send();
}

void ex::apply(account_name contract, action_name act) {
  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "must pay with ENU");
    buy(transfer);
    return;
  }

  if (contract == N(ltsonenumivo) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == LTS_SYMBOL,
                 "must pay with LTS");
    sell(transfer);
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enults(receiver);
  enults.apply(code, action);
  enumivo_exit(0);
}
}
