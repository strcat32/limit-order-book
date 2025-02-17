"""Test cases for the lob module."""
from unittest import TestCase
from .. import limit_order_book
#import limit_order_book


class ShouldInitializeLimitOrderBook(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        self.assertIsInstance(book, limit_order_book.LimitOrderBook)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())

#
# MARK: limit
#

class ShouldPlaceSellLimitOrder(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        self.assertEqual(50, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(50, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(100, book.volume_sell())
        self.assertEqual(100, book.volume_sell(50))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(100, book.volume())
        self.assertEqual(100, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(1, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(1, book.count())


class ShouldPlaceSellLimitOrderByValue(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit(False, 1, 100, 50)
        self.assertEqual(50, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(50, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(100, book.volume_sell())
        self.assertEqual(100, book.volume_sell(50))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(100, book.volume())
        self.assertEqual(100, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(1, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(1, book.count())


class ShouldPlaceBuyLimitOrder(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_buy(1, 100, 50)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(50, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(50, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(50))
        self.assertEqual(100, book.volume_buy())
        self.assertEqual(100, book.volume_buy(50))
        self.assertEqual(100, book.volume())
        self.assertEqual(100, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(1, book.count_buy())
        self.assertEqual(1, book.count())


class ShouldPlaceBuyLimitOrderByValue(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit(True, 1, 100, 50)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(50, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(50, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(50))
        self.assertEqual(100, book.volume_buy())
        self.assertEqual(100, book.volume_buy(50))
        self.assertEqual(100, book.volume())
        self.assertEqual(100, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(1, book.count_buy())
        self.assertEqual(1, book.count())

#
# MARK: limit match
#

class ShouldMatchSellLimitOrderWithIncomingBuy(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        book.limit_buy(2, 100, 50)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(50))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(50))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(50))
        self.assertEqual(0, book.count_at(50))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldMatchBuyLimitOrderWithIncomingSell(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_buy(1, 100, 50)
        book.limit_sell(2, 100, 50)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(50))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(50))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(50))
        self.assertEqual(0, book.count_at(50))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())

#
# MARK: cancel
#

class ShouldCancelSellLimitOrder(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        self.assertTrue(book.has(1))
        book.cancel(1)
        self.assertFalse(book.has(1))
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldCancelBuyLimitOrder(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_buy(1, 100, 50)
        self.assertTrue(book.has(1))
        book.cancel(1)
        self.assertFalse(book.has(1))
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())

#
# MARK: market
#

class ShouldPlaceSellMarketOrderEmptyBook(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.market_sell(1, 100)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldPlaceBuyMarketOrderEmptyBook(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.market_buy(1, 100)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldPlaceSellMarketOrderAndMatch(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_buy(1, 100, 50)
        book.market_sell(1, 10)
        self.assertEqual(0, book.best_sell())
        self.assertEqual(50, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(50, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(90, book.volume_buy())
        self.assertEqual(90, book.volume_buy(50))
        self.assertEqual(90, book.volume())
        self.assertEqual(90, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(1, book.count_buy())
        self.assertEqual(1, book.count())


class ShouldPlaceBuyMarketOrderAndMatch(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        book.market_buy(1, 10)
        self.assertEqual(50, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(50, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(90, book.volume_sell())
        self.assertEqual(90, book.volume_sell(50))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(50))
        self.assertEqual(90, book.volume())
        self.assertEqual(90, book.volume(50))
        self.assertEqual(1, book.count_at(50))
        self.assertEqual(1, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(1, book.count())

#
# MARK: clear
#

class ShouldClearSellLimitOrders(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        book.limit_sell(2, 100, 50)
        book.limit_sell(3, 100, 50)
        self.assertTrue(book.has(1))
        self.assertTrue(book.has(2))
        self.assertTrue(book.has(3))
        book.clear()
        self.assertFalse(book.has(1))
        self.assertFalse(book.has(2))
        self.assertFalse(book.has(3))
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldClearBuyLimitOrders(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()
        book.limit_sell(1, 100, 50)
        book.limit_sell(2, 100, 50)
        book.limit_sell(3, 100, 50)
        self.assertTrue(book.has(1))
        self.assertTrue(book.has(2))
        self.assertTrue(book.has(3))
        book.clear()
        self.assertFalse(book.has(1))
        self.assertFalse(book.has(2))
        self.assertFalse(book.has(3))
        self.assertEqual(0, book.best_sell())
        self.assertEqual(0, book.best_buy())
        self.assertEqual(0, book.best(False))
        self.assertEqual(0, book.best(True))
        self.assertEqual(0, book.volume_sell())
        self.assertEqual(0, book.volume_sell(100))
        self.assertEqual(0, book.volume_buy())
        self.assertEqual(0, book.volume_buy(100))
        self.assertEqual(0, book.volume())
        self.assertEqual(0, book.volume(100))
        self.assertEqual(0, book.count_at(100))
        self.assertEqual(0, book.count_sell())
        self.assertEqual(0, book.count_buy())
        self.assertEqual(0, book.count())


class ShouldModifyOrders(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()

        order_id = 1
        old_price = 50
        old_quantity = 100
        old_side = False

        new_price = 60
        new_quantity_reduce = 90
        new_quantity_increase = 110
        new_side = True

        book.limit_sell(order_id, old_quantity, old_price)

        # price
        self.assertEqual(old_quantity, book.volume(old_price))
        book.modify(1, old_side, old_quantity, new_price)
        self.assertEqual(new_price, book.get_price(order_id))
        self.assertEqual(old_quantity, book.volume(new_price))

        # side
        book.modify(1, new_side, old_quantity, new_price)
        self.assertEqual(new_side, book.get_side(order_id))
        self.assertEqual(old_quantity, book.volume(new_price))

        # reduce
        book.modify(1, new_side, new_quantity_reduce, new_price)
        self.assertEqual(new_quantity_reduce, book.volume(new_price))
        # increase
        book.modify(1, new_side, new_quantity_increase, new_price)
        self.assertEqual(new_quantity_increase, book.volume(new_price))

        self.assertEqual(new_price, book.best_buy())

        self.assertEqual(0, book.count_sell())
        self.assertEqual(1, book.count_buy())
        self.assertEqual(1, book.count())


class ShouldModifyOrders2(TestCase):
    def test(self):
        book = limit_order_book.LimitOrderBook()

        book.limit_sell(1, 1, 101)
        book.limit_sell(2, 1, 100)
        book.limit_buy(3, 1, 99)
        book.limit_buy(4, 1, 98)

        self.assertEqual(book.best_buy(), 99)
        self.assertEqual(book.best_sell(), 100)

        #book.modify(1, new_side, new_quantity_reduce, new_price)

