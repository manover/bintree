#!/usr/bin/env python

class KeyNotFound(KeyError):
    def __init__(self, last_node):
        self.last_node = last_node

class KeyPresent(RuntimeError):
    def __init__(self, node):
        self.node = node        

class RotateError(RuntimeError):
    pass

class DeleteError(RuntimeError):
    pass

class BalanceError(ValueError):
    pass

def sign(num):
    """ Returns either 1 or -1 depending on the argument sign """
    if num == 0:
        return 1
    else:
        return num / abs(num)
        
class Node(object):
    def __init__(self, key, left=None, right=None, parent=None):
        self.left = left
        self.right = right
        self.key = key
        self.parent = parent
        self.bf = 0
        
    def __contains__(self, key):
        try:
            self.search(key)
        except KeyError:
            return False
        else:
            return True
        
    def __len__(self):
        l = 1
        if self.left:
            l += self.left.__len__()
        if self.right:
            l += self.right.__len__()
            
        return l
    
    def __nonzero__(self):
        return True
    
    def to_dict(self, d=None):
        if d is None:
            d = {self.key: self}
        else:
            d[self.key] = self
        if self.left:
            self.left.to_dict(d)
        if self.right:
            self.right.to_dict(d)
            
        return d
        
    def traverse(self, f):
        f(self)
        if self.left:
            self.left.traverse(f)
        if  self.right:
            self.right.traverse(f)
            
    def check(self):
        print "%d: calc_balance = %d, bf = %d" % \
                    (self.key, self.calc_bf(), self.bf),
        if self.bf != self.calc_bf():
            print "!!! BAD !!! ",
            
        if self.right:
            if self.right.parent is not self:
                print "!!! right.parent is not self !!! ",
            if self.right.key <= self.key:
                print "right.key <= self.key"
        if self.left:
            if self.left.parent is not self:
                print "!!! left.parent is not self !!! ",
            if self.left.key >= self.key:
                print "!!! left.key >= self.key !!! ",
        print
    
    def _search(self, key):
        """
            Returns the corresponding node if found, the last checked otherwise
        """
        n = self
        while n is not None:
            last = n
            if n.key > key:
                n = n.left
            elif n.key < key:
                n = n.right
            else:
                return n
        return last
    
    def search(self, key):
        n = self._search(key)
        if n.key == key:
            return n
        else:
            raise KeyNotFound(n)
    
    def get_child_place(self, child):
        """ Returns 1 if child is on the left subtree, -1 if on the right """
        if child.key < self.key:
            return 1
        else:
            return -1
    
    def rebalance(self):
        return
    
    def update_bf_on_insert(self, delta, dont_rebalance=False):
        """
            Updates balance factor for the node and all its
            ancestors on insert
        """
        #print "update_bf_on_insert: %d: delta = %d, dont_rebalance=%s, bf = %d" % \
        #                            (self.key, delta, dont_rebalance, self.bf)
        self.bf += delta
        bf = self.bf
        
        if bf == 0 or sign(bf) != sign(delta):
            # Subtree height did not change
            return
        else:
            # Subtree height increased
            parent = self.parent
            if parent:
                # Set don't rebalance flag for future updates if this node needs
                # rotation, it will be taken care of below
                parent.update_bf_on_insert(parent.get_child_place(self),
                              dont_rebalance=(abs(bf) > 1 or dont_rebalance))
        if abs(bf) > 1 and not dont_rebalance:
            self.rebalance()
                    
                
    def update_bf_on_delete(self, delta, dont_rebalance=False):
        """
            Updates balance factor for the node and all its
            ancestors on delete
        """
        self.bf += delta
        bf = self.bf
        
        if bf == 0 or sign(bf) != sign(delta):
            # Subtree height decreased
            parent = self.parent
            if parent:
                parent.update_bf_on_delete(-parent.get_child_place(self),
                               dont_rebalance=(abs(bf) > 1 or dont_rebalance))
        if abs(bf) > 1 and not dont_rebalance:
            self.rebalance()
            

    def insert(self, key):
        p = self._search(key)
        if p.key == key:
            raise KeyPresent(p)
        else:
            node = self.__class__(key)
            p.update_bf_on_insert(node.connect_to_parent(p))

    def disconnect(self, node):
        if self.left is node:
            self.left = None
        else:
            self.right = None
    
    def delete(self, key):
        node = self.search(key)
        p = node.parent

        if node.left and node.right:
            nkey = node.left.rightmost().key
            node.left.delete(nkey)
            node.key = nkey
        else:
            if p:
                bf = p.get_child_place(node)

            if node.left:
                node.left.connect_to_parent(p)
            elif node.right:
                node.right.connect_to_parent(p)
            else:
                # No children
                p.disconnect(node)

            if p:
                # Update ancestors' bf
                p.update_bf_on_delete(-bf)
            

    def calc_heights(self):
        if self.left:
            h_left =  self.left.height()
        else:
            h_left = 0
            
        if self.right:
            h_right = self.right.height()
        else:
            h_right = 0
            
        return h_left, h_right
    
    def calc_bf(self):
        h_left, h_right = self.calc_heights()
        return h_left - h_right
    
    def height(self):            
        return 1 + max(self.calc_heights())
    
    def to_list(self):
        left = None
        right = None
        if self.left:
            left = self.left.to_list()
        if self.right:
            right = self.right.to_list()
        return [self.key, left, right]

    @classmethod
    def from_list(cls, l, parent=None):
        if l:
            tree = cls(l[0], parent=parent)
            for i in l[1:]:
                tree.insert(i)
            
            return tree
        else:
            return None
    
    @classmethod
    def from_list_raw(cls, l, parent=None):           
        key = l[0]
        left = None
        right = None
        node = cls(key, parent=parent)
        if l[1]:
            left = cls.from_list_raw(l[1], parent=node)
        if l[2]:
            right = cls.from_list_raw(l[2], parent=node)

        node.left = left
        node.right = right

        node.bf = node.calc_bf()
        return node

    def __eq__(self, t2):
        return self.to_list() == t2.to_list()

    def __repr__(self):
        left = repr(self.left)
        right = repr(self.right)
        return "%d(%s,%s)" % (self.key, left, right)

    def rightmost(self):
        """ Return the rightmost node """
        if self.right:
            return self.right.rightmost()
        else:
            return self

    def connect(self, node):
        bf = self.get_child_place(node)
        if bf == 1:
            self.left = node
        else:
            self.right = node
        return bf
    
    def connect_to_parent(self, parent):
        bf = parent.connect(self)
        self.parent = parent
        
        return bf

    def rotate_cw(self):
        """
                    PARENT                  PARENT
                    /    \                  /     \
                RIGHT           =>      PIVOT
                /   \                   /   \
            PIVOT    A              LEFT    RIGHT
            /   \                           /    \
        LEFT     B                         B      A

        """
        
        pivot = self
        right = self.parent        
        parent = right.parent
        
        if right:
            if right.right is self:
                raise RotateError("Unable to rotate right subtree CW")
        else:
            raise RotateError("Unable to rotate root")
            
        # Save old subtree bf
        old_bf = right.bf
        
        # Connect PIVOT to PARENT
        pivot.connect_to_parent(parent)
        if pivot.right:
            # Connect B to RIGHT
            pivot.right.connect_to_parent(right)
        else:
            right.left = None
            
        # Connect RIGHT to PIVOT
        right.connect_to_parent(pivot)
        
        # Update bf's
        # RIGHT's left subtree is 1 node shorter now (minus PIVOT)
        right.bf -= 1
        if pivot.bf > 0:
            # If PIVOT bf > 0 PIVOT subtree height was represented by LEFT
            # LEFT is no longer RIGHT's successor, so compensate for it
            right.bf -= pivot.bf

        # PIVOT's right subtree is 1 node longer now (plus RIGHT)
        pivot.bf -= 1
        if right.bf < 0:
            # If RIGHT bf < 0 RIGHT subtree height is represented by A
            # A is now PIVOT's successor, update bf accordingly
            pivot.bf += right.bf
            
        delta = abs(pivot.bf) - abs(old_bf)
        if parent:
            # When rotating, every height change in one node is accounted
            # for double change in bf, e.g. when rotating tree with bf = 2 CW,
            # the new bf will be 0, height will decrease by 1 
            if delta > 0:
                # Subtree height increased
                parent.update_bf_on_insert(delta/2 * parent.get_child_place(pivot))
            elif delta < 0:
                # Subtree height decreased
                parent.update_bf_on_delete(delta/2 * parent.get_child_place(pivot))
                

    def rotate_ccw(self):
        """
        PARENT                    PARENT
        /    \                   /     \
             LEFT         =>          PIVOT
            /    \                    /   \
           A    PIVOT              LEFT    RIGHT
               /     \            /    \
              B     RIGHT        A      B

        """

        pivot = self
        left = self.parent
        parent = left.parent

        if left:
            if left.left is self:
                raise RotateError("Unable to rotate left subtree CCW")
        else:
            raise RotateError("Unable to rotate root")
            
        old_bf = left.bf
        # Connect PIVOT to PARENT
        pivot.connect_to_parent(parent)
        if pivot.left:
            # Connect B to LEFT
            pivot.left.connect_to_parent(left)
        else:
            left.right = None
        # Connect LEFT to PIVOT
        left.connect_to_parent(pivot)
        
        # Update bf's
        # LEFT's right subtree is 1 node shorter now (minus PIVOT)
        left.bf += 1
        if pivot.bf < 0:
            # If PIVOT bf < 0 PIVOT subtree height was represented by RIGHT
            # RIGHT is no longer LEFT's successor, so compensate for it
            left.bf -= pivot.bf

        # PIVOT's left subtree is 1 node longer now (plus LEFT)
        pivot.bf += 1
        if left.bf > 0:
            # If LEFT bf > 0 LEFT subtree height is represented by A
            # A is now PIVOT's successor, update bf accordingly
            pivot.bf += left.bf
            
        delta = abs(pivot.bf) - abs(old_bf)
        if parent:
            # When rotating, every height change in one node is accounted
            # for double change in bf, e.g. when rotating tree with bf = 2 CW,
            # the new bf will be 0, height will decrease by 1 
            if delta > 1:
                # Subtree height increased
                parent.update_bf_on_insert(delta/2 * parent.get_child_place(pivot))
            elif delta < -1:
                # Subtree height decreased
                parent.update_bf_on_delete(delta/2 * parent.get_child_place(pivot))
        
class AVL(Node):
    def rebalance(self):
        if self.bf == 2:
            if self.left.bf >= 0:
                #print "%d: Left-left case" % self.key
                # Left left case
                self.left.rotate_cw()
            else:
                #print "%d: Left-right case" % self.key
                # Left right case
                self.left.right.rotate_ccw()
                self.left.rotate_cw()
        elif self.bf == -2:
            if self.right.bf <= 0:
                #print "%d: Right-right case" % self.key
                # Right right case
                self.right.rotate_ccw()
            else:
                #print "%d: Right-left case" % self.key
                # Right left case
                self.right.left.rotate_cw()
                self.right.rotate_ccw()
        else:
            raise BalanceError("Unable to balance node %d, bf = %d" %
                               (self.key, self.bf))
class Tree(object):
    """
        Tree wrapper object
    """
    _node_class = Node
    
    def __init__(self, root=None):
        self.root = root
        if root:
            root.parent = self
            
    def __contains__(self, key):
        if self.root:
            return key in self.root
        else:
            return False
        
    def __len__(self):
        if self.root:
            return len(self.root)
        else:
            return 0
            
    def connect(self, node):
        self.root = node
        return 0
    
    def disconnect(self, node):
        self.root = None
    
    def height(self):
        if self.root:
            return self.root.height()
        else:
            return 0
        
    def insert(self, key):
        if self.root:
            self.root.insert(key)
        else:
            self.root = self._node_class(key)
            self.root.parent = self
            
    def delete(self, key):
        if self.root:
            return self.root.delete(key)
        else:
            raise KeyNotFound(None)
            
    def traverse(self, f):
        if self.root:
            self.root.traverse(f)
            
    def search(self, key):
        if self.root:
            return self.root.search(key)
        else:
            raise KeyNotFound(None)
            
    def __nonzero__(self):
        return False
    
    def to_list(self):
        if self.root:
            return self.root.to_list()
        else:
            return []
            
    def to_dict(self):
        if self.root:
            return self.root.to_dict()
        else:
            return {}
            
    @classmethod
    def from_list(cls, l):
        t = cls()
        for i in l:
            t.insert(i)
        return t
        
    @classmethod
    def from_list_raw(cls, l):
        t = cls._node_class.from_list_raw(l, cls())
        t.parent.root = t
        return t.parent

class BalancedTree(Tree):
    _node_class = AVL
