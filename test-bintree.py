#!/usr/bin/env python

import unittest
import random

from bintree import *

class TestCase(unittest.TestCase):
    LIST = [6, [4, [1, [0, None, None], [3, None, None]], None], [7, None, [9, None, [12, None, None]]]]
    def setUp(self):
        self.tree = Tree.from_list_raw(self.LIST)

    def check(self, node):
        #print "%d: %d == %d" % (node.key, node.bf, node.calc_bf())
        self.assertEqual(node.bf, node.calc_bf())
        if node.left:
            self.assertIs(node, node.left.parent)
            self.assertGreater(node.key, node.left.key)
        if node.right:
            self.assertIs(node, node.right.parent)
            self.assertLess(node.key, node.right.key)

    def test_01_from_list(self):
        tree = self.tree
        self.assertEqual(tree.to_list(), self.LIST)

    def test_02_search(self):
        tree = self.tree
        n = tree.search(12)
        p = n.parent

        self.assertTrue(p)
        self.assertEqual(n.key, 12)
        self.assertEqual(p.key, 9)

    def test_03_insert(self):
        tree = Tree()
        for i in (6, 4, 7, 9, 12, 1, 0, 3):
            tree.insert(i)

        self.assertEqual(tree.to_list(), self.LIST)

    def test_04_delete(self):
        tree = self.tree

        tree.delete(12)
        self.assertEqual(tree.to_list(),
            [6, [4, [1, [0, None, None], [3, None, None]], None], [7, None, [9, None, None]]])
        tree.traverse(self.check)

        tree.delete(7)
        self.assertEqual(tree.to_list(),
            [6, [4, [1, [0, None, None], [3, None, None]], None], [9, None, None]])
        tree.traverse(self.check)

        tree.delete(1)
        self.assertEqual(tree.to_list(),
            [6, [4, [0, None, [3, None, None]], None], [9, None, None]])
        tree.traverse(self.check)

        tree.delete(6)
        self.assertEqual(tree.to_list(),
            [4, [0, None, [3, None, None]], [9, None, None]])
        tree.traverse(self.check)

        tree.delete(9)
        self.assertEqual(tree.to_list(),
            [4, [0, None, [3, None, None]], None])
        tree.traverse(self.check)
        
        tree = Tree.from_list_raw([150, [130, None, None], [170, None, [190, None, [210, None, None]]]])
        tree.delete(170)
        self.assertEqual(tree.to_list(),
            [150, [130, None, None], [190, None, [210, None, None]]])
        tree.traverse(self.check)
        
        tree.delete(210)
        self.assertEqual(tree.to_list(),
            [150, [130, None, None], [190, None, None]])
        tree.traverse(self.check)

    def test_05_rightmost(self):
        tree = self.tree.root

        self.assertEqual(tree.rightmost().key, 12)
        self.assertEqual(tree.search(4).rightmost().key, 4)
        self.assertEqual(tree.search(1).rightmost().key, 3)
        self.assertEqual(tree.search(7).rightmost().key, 12)

    def test_06_rotate(self):
        tree = Tree.from_list([100, 50, 40, 60, 150, 170, 190, 200])

        tree.search(40).rotate_cw()
        self.assertEqual(tree.to_list(),
            [100, [40, None, [50, None, [60, None, None]]],
             [150, None, [170, None, [190, None, [200, None, None]]]]]
        )
        tree.traverse(self.check)

        tree.search(170).rotate_ccw()
        self.assertEqual(tree.to_list(),
            [100, [40, None, [50, None, [60, None, None]]],
             [170, [150, None, None], [190, None, [200, None, None]]]]
        )
        tree.traverse(self.check)
        
        t1 = Tree.from_list_raw([32, [23, None, None], [48, None, [59, None, None]]])
        t1.root.right.rotate_ccw()
        t1.traverse(self.check)
        
    def test_07_height(self):
        tree = Tree.from_list([100])
        self.assertEqual(tree.height(), 1)        
        tree.insert(50)
        self.assertEqual(tree.height(), 2)
        tree.insert(30)
        self.assertEqual(tree.height(), 3)
        tree.insert(60)
        self.assertEqual(tree.height(), 3)        
        tree.insert(70)
        self.assertEqual(tree.height(), 4)
        
    def test_08_calc_bf(self):
        tree = Node(100)
        self.assertEqual(tree.calc_bf(), 0)    
        tree.insert(50)
        self.assertEqual(tree.calc_bf(), 1)
        tree.insert(30)
        self.assertEqual(tree.calc_bf(), 2)
        tree.insert(60)
        self.assertEqual(tree.calc_bf(), 2)
        tree.insert(150)
        self.assertEqual(tree.calc_bf(), 1)
        tree.insert(170)
        self.assertEqual(tree.calc_bf(), 0)
        tree.insert(130)
        self.assertEqual(tree.calc_bf(), 0)
        
    def test_09_bf(self):        
        l = [50, 30, 60, 150, 170, 130, 190, 210, 15, 10]
        tree = Tree.from_list([100])
        for i in l:
            tree.insert(i)
        tree.traverse(self.check)
        for i in [60, 170, 210, 15]:
            tree.delete(i)
        tree.traverse(self.check)
        
    def test_10_invalid_rotation(self):
        tree = Tree.from_list([100, 30, 20, 50])
        self.assertRaises(RotateError, tree.search(20).rotate_ccw)
        self.assertRaises(RotateError, tree.search(50).rotate_cw)
        
    def test_11_random(self):
        l = [88, 69, 68, 83, 24, 37, 96, 38, 53, 31, 4, 82, 10, 77, 59, 79, 32, 65, 23, 48]
        #random.sample(xrange(100), 40)
        tree = BalancedTree.from_list(l)
        tree.traverse(self.check)
        for i in l[:-1]:
            #print "delete %d" % i
            tree.delete(i)
            tree.traverse(self.check)
        l2 = [25, 94, 43, 82, 11, 32, 14, 22, 74, 65, 5, 0, 2, 68, 89, 40, 19, 31, 8, 49, 96,
              58, 10, 1, 36, 60, 28, 41, 84, 30, 83, 12, 77, 86, 18, 45, 26, 44, 53, 66]
        cl = [48]
        for i in l2[:30]:
            tree.insert(i)
            cl.append(i)
            tree.traverse(self.check)
            self.assertItemsEqual(tree.to_dict().keys(), cl)
        for i in l2[10:30]:
            #print "delete >> %d" % i
            del cl[cl.index(i)]
            tree.delete(i)
            tree.traverse(self.check)
            self.assertItemsEqual(tree.to_dict().keys(), cl)
        for i in l2[30:]:
            cl.append(i)
            tree.insert(i)
            tree.traverse(self.check)
            self.assertItemsEqual(tree.to_dict().keys(), cl)
        for i in l2[:10]+l2[30:]:
            self.assertIn(i, tree)
            
    def test_12_insert(self):
        sl = [68, [58, [49, None, None], [65, None, None]], [74, None, None]]
        t = BalancedTree.from_list_raw(sl)
        t.insert(60)
        t.traverse(self.check)

import random

if __name__ == "__main__":
    unittest.main()
