# Generated from MultFirst.g4 by ANTLR 4.7
# encoding: utf-8
from antlr4 import *
from io import StringIO
from typing.io import TextIO
import sys

def serializedATN():
    with StringIO() as buf:
        buf.write("\3\u608b\ua72a\u8133\ub9ed\u417c\u3be7\u7786\u5964\3\t")
        buf.write("\31\4\2\t\2\3\2\3\2\3\2\3\2\3\2\3\2\3\2\5\2\f\n\2\3\2")
        buf.write("\3\2\3\2\3\2\3\2\3\2\7\2\24\n\2\f\2\16\2\27\13\2\3\2\2")
        buf.write("\3\2\3\2\2\2\2\33\2\13\3\2\2\2\4\5\b\2\1\2\5\6\7\3\2\2")
        buf.write("\6\7\5\2\2\2\7\b\7\4\2\2\b\f\3\2\2\2\t\f\7\b\2\2\n\f\7")
        buf.write("\7\2\2\13\4\3\2\2\2\13\t\3\2\2\2\13\n\3\2\2\2\f\25\3\2")
        buf.write("\2\2\r\16\f\7\2\2\16\17\7\6\2\2\17\24\5\2\2\b\20\21\f")
        buf.write("\6\2\2\21\22\7\5\2\2\22\24\5\2\2\7\23\r\3\2\2\2\23\20")
        buf.write("\3\2\2\2\24\27\3\2\2\2\25\23\3\2\2\2\25\26\3\2\2\2\26")
        buf.write("\3\3\2\2\2\27\25\3\2\2\2\5\13\23\25")
        return buf.getvalue()


class MultFirstParser ( Parser ):

    grammarFileName = "MultFirst.g4"

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]

    sharedContextCache = PredictionContextCache()

    literalNames = [ "<INVALID>", "'('", "')'", "'+'", "'*'" ]

    symbolicNames = [ "<INVALID>", "LeftParen", "RightParen", "Plus", "Multiply", 
                      "Number", "Identifier", "WhiteSpace" ]

    RULE_exp = 0

    ruleNames =  [ "exp" ]

    EOF = Token.EOF
    LeftParen=1
    RightParen=2
    Plus=3
    Multiply=4
    Number=5
    Identifier=6
    WhiteSpace=7

    def __init__(self, input:TokenStream, output:TextIO = sys.stdout):
        super().__init__(input, output)
        self.checkVersion("4.7")
        self._interp = ParserATNSimulator(self, self.atn, self.decisionsToDFA, self.sharedContextCache)
        self._predicates = None



    class ExpContext(ParserRuleContext):

        def __init__(self, parser, parent:ParserRuleContext=None, invokingState:int=-1):
            super().__init__(parent, invokingState)
            self.parser = parser


        def getRuleIndex(self):
            return MultFirstParser.RULE_exp

     
        def copyFrom(self, ctx:ParserRuleContext):
            super().copyFrom(ctx)


    class MultContext(ExpContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a MultFirstParser.ExpContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def exp(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(MultFirstParser.ExpContext)
            else:
                return self.getTypedRuleContext(MultFirstParser.ExpContext,i)

        def Multiply(self):
            return self.getToken(MultFirstParser.Multiply, 0)

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterMult" ):
                listener.enterMult(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitMult" ):
                listener.exitMult(self)


    class NumContext(ExpContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a MultFirstParser.ExpContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def Number(self):
            return self.getToken(MultFirstParser.Number, 0)

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterNum" ):
                listener.enterNum(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitNum" ):
                listener.exitNum(self)


    class IdContext(ExpContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a MultFirstParser.ExpContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def Identifier(self):
            return self.getToken(MultFirstParser.Identifier, 0)

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterId" ):
                listener.enterId(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitId" ):
                listener.exitId(self)


    class PlusContext(ExpContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a MultFirstParser.ExpContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def exp(self, i:int=None):
            if i is None:
                return self.getTypedRuleContexts(MultFirstParser.ExpContext)
            else:
                return self.getTypedRuleContext(MultFirstParser.ExpContext,i)

        def Plus(self):
            return self.getToken(MultFirstParser.Plus, 0)

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterPlus" ):
                listener.enterPlus(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitPlus" ):
                listener.exitPlus(self)


    class BracContext(ExpContext):

        def __init__(self, parser, ctx:ParserRuleContext): # actually a MultFirstParser.ExpContext
            super().__init__(parser)
            self.copyFrom(ctx)

        def LeftParen(self):
            return self.getToken(MultFirstParser.LeftParen, 0)
        def exp(self):
            return self.getTypedRuleContext(MultFirstParser.ExpContext,0)

        def RightParen(self):
            return self.getToken(MultFirstParser.RightParen, 0)

        def enterRule(self, listener:ParseTreeListener):
            if hasattr( listener, "enterBrac" ):
                listener.enterBrac(self)

        def exitRule(self, listener:ParseTreeListener):
            if hasattr( listener, "exitBrac" ):
                listener.exitBrac(self)



    def exp(self, _p:int=0):
        _parentctx = self._ctx
        _parentState = self.state
        localctx = MultFirstParser.ExpContext(self, self._ctx, _parentState)
        _prevctx = localctx
        _startState = 0
        self.enterRecursionRule(localctx, 0, self.RULE_exp, _p)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 9
            self._errHandler.sync(self)
            token = self._input.LA(1)
            if token in [MultFirstParser.LeftParen]:
                localctx = MultFirstParser.BracContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx

                self.state = 3
                self.match(MultFirstParser.LeftParen)
                self.state = 4
                self.exp(0)
                self.state = 5
                self.match(MultFirstParser.RightParen)
                pass
            elif token in [MultFirstParser.Identifier]:
                localctx = MultFirstParser.IdContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx
                self.state = 7
                self.match(MultFirstParser.Identifier)
                pass
            elif token in [MultFirstParser.Number]:
                localctx = MultFirstParser.NumContext(self, localctx)
                self._ctx = localctx
                _prevctx = localctx
                self.state = 8
                self.match(MultFirstParser.Number)
                pass
            else:
                raise NoViableAltException(self)

            self._ctx.stop = self._input.LT(-1)
            self.state = 19
            self._errHandler.sync(self)
            _alt = self._interp.adaptivePredict(self._input,2,self._ctx)
            while _alt!=2 and _alt!=ATN.INVALID_ALT_NUMBER:
                if _alt==1:
                    if self._parseListeners is not None:
                        self.triggerExitRuleEvent()
                    _prevctx = localctx
                    self.state = 17
                    self._errHandler.sync(self)
                    la_ = self._interp.adaptivePredict(self._input,1,self._ctx)
                    if la_ == 1:
                        localctx = MultFirstParser.MultContext(self, MultFirstParser.ExpContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_exp)
                        self.state = 11
                        if not self.precpred(self._ctx, 5):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 5)")
                        self.state = 12
                        self.match(MultFirstParser.Multiply)
                        self.state = 13
                        self.exp(6)
                        pass

                    elif la_ == 2:
                        localctx = MultFirstParser.PlusContext(self, MultFirstParser.ExpContext(self, _parentctx, _parentState))
                        self.pushNewRecursionContext(localctx, _startState, self.RULE_exp)
                        self.state = 14
                        if not self.precpred(self._ctx, 4):
                            from antlr4.error.Errors import FailedPredicateException
                            raise FailedPredicateException(self, "self.precpred(self._ctx, 4)")
                        self.state = 15
                        self.match(MultFirstParser.Plus)
                        self.state = 16
                        self.exp(5)
                        pass

             
                self.state = 21
                self._errHandler.sync(self)
                _alt = self._interp.adaptivePredict(self._input,2,self._ctx)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.unrollRecursionContexts(_parentctx)
        return localctx



    def sempred(self, localctx:RuleContext, ruleIndex:int, predIndex:int):
        if self._predicates == None:
            self._predicates = dict()
        self._predicates[0] = self.exp_sempred
        pred = self._predicates.get(ruleIndex, None)
        if pred is None:
            raise Exception("No predicate with index:" + str(ruleIndex))
        else:
            return pred(localctx, predIndex)

    def exp_sempred(self, localctx:ExpContext, predIndex:int):
            if predIndex == 0:
                return self.precpred(self._ctx, 5)
         

            if predIndex == 1:
                return self.precpred(self._ctx, 4)
         




