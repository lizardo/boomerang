/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   proc.h
 * OVERVIEW:   Interface for the procedure classes, which are used to
 *			   store information about variables in the procedure such
 *			   as parameters and locals.
 *============================================================================*/

/* $Revision: 1.99.2.2 $
 * 20 Sep 01 - Brian: Added getSymbolicLocals() to return the list of symbolic
 *				locals for a procedure.
*/

#ifndef _PROC_H_
#define _PROC_H_

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <assert.h>
#include "exp.h"				// For lessExpStar
#include "cfg.h"				// For cfg->simplify()
#include "hllcode.h"
#include "memo.h"

class Prog;
class UserProc;
class Cfg;
class BasicBlock;
typedef BasicBlock* PBB;
class Exp;
class TypedExp;
class lessTI;
class Type;
class RTL;
class HLLCode;
class HLCall;
class Parameter;
class Argument;
class Signature;
class Cluster;
class XMLProgParser;

/*==============================================================================
 * Procedure class.
 *============================================================================*/
class Proc : public Memoisable {
public:

    /*
     * Constructor with name, native address and optional bLib.
     */
    Proc(Prog *prog, ADDRESS uNative, Signature *sig);

    virtual ~Proc();

    /*
     * Gets name of this procedure.
     */
    const char* getName();

    /*
     * Gets sets the name of this procedure.
     */
    void setName(const char *nam);

    /*
     * Get the native address.
     */
    ADDRESS getNativeAddress();

    /*
     * Set the native address
     */
    void setNativeAddress(ADDRESS a);

    /*
     * Get the program this procedure belongs to.
     */
    Prog *getProg();
    void setProg(Prog *p) {
        prog = p;
    }

    /*
     * Get/Set the first procedure that calls this procedure (or null for main/start).
     */
    Proc *getFirstCaller();
    void setFirstCaller(Proc *p) {
        if (m_firstCaller == NULL) m_firstCaller = p;
    }

    /*
     * Returns a pointer to the Signature
     */
    Signature *getSignature() {
        return signature;
    }
    void setSignature(Signature *sig) {
        signature = sig;
    }

    virtual void renameParam(const char *oldName, const char *newName);

    /*
     * Prints this procedure to an output stream.
     */
    virtual std::ostream& put(std::ostream& os) = 0;

    /*
     * Modify actuals so that it is now the list of locations that must
     * be passed to this procedure. The modification will be to either add
     * dummp locations to actuals, delete from actuals, or leave it
     * unchanged.
     * Add "dummy" params: this will be required when there are
     *	 less live outs at a call site than the number of parameters
     *	 expected by the procedure called. This will be a result of
     *	 one of two things:
     *	 i) a value returned by a preceeding call is used as a
     *		parameter and as such is not detected as defined by the
     *		procedure. E.g.:
     *
     *		   foo(bar(x));
     *
     *		Here, the result of bar(x) is used as the first and only
     *		parameter to foo. On some architectures (such as SPARC),
     *		the location used as the first parameter (e.g. %o0) is
     *		also the location in which a value is returned. So, the
     *		call to bar defines this location implicitly as shown in
     *		the following SPARC assembly that may be generated by from
     *		the above code:
     *
     *			mov	  x, %o0
     *			call  bar
     *			nop
     *			call  foo
     *
     *	   As can be seen, there is no definition of %o0 after the
     *	   call to bar and before the call to foo. Adding the integer
     *	   return location is therefore a good guess for the dummy
     *	   location to add (but may occasionally be wrong).
     *
     *	ii) uninitialised variables are used as parameters to a call
     *
     *	Note that both of these situations can only occur on
     *	architectures such as SPARC that use registers for parameter
     *	passing. Stack parameters must always be pushed so that the
     *	callee doesn't access the caller's non-parameter portion of
     *	stack.
     *
     * This used to be a virtual function, implemented differenty for
     * LibProcs and for UserProcs. But in fact, both need the exact same
     * treatment; the only difference is how the local member "parameters"
     * is set (from common.hs in the case of LibProc objects, or from analysis
     * in the case of UserProcs).
     */
    void matchParams(std::list<Exp*>& actuals, UserProc& caller);

    /*
     * Get a list of types to cast a given list of actual parameters to
     */
    std::list<Type>* Proc::getParamTypeList(const std::list<Exp*>& actuals);

    /*
     * Return true if this is a library proc
     */
    virtual bool isLib() {
        return false;
    }

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() {
        return false;
    }

    /*
     * OutPut operator for a Proc object.
     */
    friend std::ostream& operator<<(std::ostream& os, Proc& proc);

    virtual Exp *getProven(Exp *left) = 0;

    // Set an equation as proven. Useful for some sorts of testing
    void setProven(Exp* fact) {
        proven.insert(fact);
    }

    /*
     * Get the callers
     * Note: the callers will be in a random order (determined by memory
     * allocation)
     */
    std::set<CallStatement*>& getCallers() {
        return callerSet;
    }

    /*
     * Add to the set of callers
     */
    void addCaller(CallStatement* caller) {
        callerSet.insert(caller);
    }

    /*
     * Add to a set of caller Procs
     */
    void addCallers(std::set<UserProc*>& callers);

    virtual void removeReturn(Exp *e);
    void removeParameter(Exp *e);
    void addParameter(Exp *e);
    virtual void addReturn(Exp *e);

    virtual void printCallGraphXML(std::ostream &os, int depth,
                                   bool recurse = true);
    void printDetailsXML();
    void clearVisited() {
        visited = false;
    }
    bool isVisited() {
        return visited;
    }

    Cluster *getCluster() {
        return cluster;
    }
    void setCluster(Cluster *c) {
        cluster = c;
    }

    virtual Memo *makeMemo(int mId) = 0;
    virtual void readMemo(Memo *m, bool dec) = 0;

protected:

    bool visited;

    Prog *prog;								// Program containing this procedure.

    /*
     * The formal signature of this procedure. This information is determined
     * either by the common.hs file (for a library function) or by analysis.
     */
    Signature *signature;

    /* Persistent state */
    ADDRESS address;						// Procedure's address.
    Proc *m_firstCaller;					// first procedure to call this procedure.
    ADDRESS m_firstCallerAddr;				// can only be used once.
    std::set<Exp*, lessExpStar> proven;		// all the expressions that have been proven
    std::set<CallStatement*> callerSet;		// Set of callers (CallStatements that call this procedure).
    Cluster *cluster;				// Cluster this procedure is contained within.

    friend class XMLProgParser;
    Proc() : visited(false), prog(NULL), signature(NULL), address(0), m_firstCaller(NULL), m_firstCallerAddr(0), cluster(NULL) { }

};	// class Proc

/*==============================================================================
 * LibProc class.
 *============================================================================*/
class LibProc : public Proc {
public:

    LibProc(Prog *prog, std::string& name, ADDRESS address);
    virtual ~LibProc();

#if 0
    /*
     * See comment for Proc::matchParams.
     */
    void matchParams(std::list<Exp*>& actuals, UserProc& caller,
                     const Parameters* outgoing, const Exp** intRetLoc) const;
#endif

    /*
     * Return true, since is a library proc
     */
    bool isLib() {
        return true;
    }

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() {
        return false;
    }

    virtual Exp* getProven(Exp* left);

    /*
     * Prints this procedure to an output stream.
     */
    std::ostream& put(std::ostream& os);

    virtual Memo *makeMemo(int mId);
    virtual void readMemo(Memo *m, bool dec);

    void getInternalStatements(StatementList &internal);
protected:

    friend class XMLProgParser;
    LibProc() : Proc() { }

};		// class LibProc

/*==============================================================================
 * UserProc class.
 *============================================================================*/
class UserProc : public Proc {

    /*
     * The control flow graph.
     */
    Cfg* cfg;

    /*
     * True if this procedure has been decoded.
     */
    bool decoded;

    // true if the procedure has been analysed.
    bool analysed;

    /*
     * Indicate that the aggregate location pointer "hidden" parameter is used,
     * and is thus explicit in this translation. Needed only by architectures
     * like Sparc where a special parent stack location is used to pass the
     * address of aggregates. Set with the setParams() member function
     */
    bool aggregateUsed;

    /*
     * This map records the allocation of local variables and their types.
     */
    std::map<std::string, Type*> locals;

    /*
     * A map between machine dependent locations and their corresponding
     * symbolic, machine independent representations.
     */
    std::map<Exp*,Exp*,lessExpStar> symbolMap;

    /*
     * Set of callees (Procedures that this procedure calls). Used for
     * call graph, among other things
     */
    std::set<Proc*> calleeSet;

    /*
    * Set if visited on the way down the call tree during decompile()
    * Used for recursion detection
    */
    bool decompileSeen;

    /*
     * Set if decompilation essentially completed (there may be extra return
     * locations set later)
     */
    bool decompiled;

    /*
     * Set if involved in recursion (a cycle in the call graph)
     */
    bool isRecursive;

    /*
     * Set of locations defined in this proc. Some or all or none of these
     * may be return locations (will be if used before definition after the
     * call)
     * Note: there is a different set in each call, because the locations
     * may be different from the caller's perspective (e.g. stack locations)
     */
    LocationSet definesSet;

    /*
     * Set of locations returned by this proc (see above). As calls are found
     * with use-before-def of locations in definesSet, they are transferred
     * to this set.
     */
    LocationSet returnsSet;

public:

    UserProc(Prog *prog, std::string& name, ADDRESS address);
    virtual ~UserProc();

    /*
     * Records that this procedure has been decoded.
     */
    void setDecoded();

    /*
     * Removes the decoded bit and throws away all the current information
     * about this procedure.
     */
    void unDecode();

    /*
     * Returns a pointer to the CFG.
     */
    Cfg* getCFG();

    /*
     * Deletes the whole CFG and all the RTLs, RTs, and Exp*s associated with
     * it. Also nulls the internal cfg pointer (to prevent strange errors)
     */
    void deleteCFG();

    /*
     * Returns an abstract syntax tree for the procedure in the
     * internal representation.	 This function actually _calculates_
     * this value and is expected to do so expensively.
     */
    SyntaxNode *getAST();
    // print it to a file
    void printAST(SyntaxNode *a = NULL);

    /*
     * Returns whether or not this procedure can be decoded (i.e. has
     * it already been decoded).
     */
    bool isDecoded();
    bool isDecompiled() {
        return decompiled;
    }

    bool isAnalysed() {
        return analysed;
    }
    void setAnalysed() {
        analysed = true;
    }

    /*
     * Return the number of bytes allocated for locals on the stack.
     */
    int getLocalsSize();

    /*
     * Get the type of the given var
     */
//    Type getVarType(int idx);

    /*
     * Set the size of the given var
     */
//    void setVarSize(int idx, int size);

    // code generation
    void generateCode(HLLCode *hll);

    // print this proc, mainly for debugging
    void print(std::ostream &out);
    void printToLog();

    // simplify the statements in this proc
    void simplify() {
        cfg->simplify();
    }

    // decompile this proc
    std::set<UserProc*>* decompile();
    void propagateAtDepth(int depth);
    void updateBlockVars();

    Statement *getStmtAtLex(unsigned int begin, unsigned int end);

    // All the decompile stuff except propagation, DFA repair, and null/unused
    // statement removal
    void    complete();

    // Initialise the statements, e.g. proc, bb pointers
    void initStatements();
    void numberStatements(int& stmtNum);
    void numberPhiStatements(int& stmtNum);
    bool nameStackLocations();
    bool nameRegisters();
    void removeRedundantPhis();
    void trimReturns();
    void fixCallRefs();
    void addNewParameters();
    void addNewReturns(int depth);
    // Trim parameters. If depth not given or == -1, perform at all depths
    void trimParameters(int depth = -1);
    void processFloatConstants();
    void replaceExpressionsWithGlobals();
    void replaceExpressionsWithSymbols();
    void replaceExpressionsWithParameters(int depth);   // must be in SSA form
    void replaceExpressionsWithLocals(bool lastPass = false);

private:
    void searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp,
                             StatementList& stmts);
public:
    bool removeNullStatements();
    bool removeDeadStatements();
    typedef std::map<Statement*, int> RefCounter;
    void countRefs(RefCounter& refCounts);
    void removeUnusedStatements(RefCounter& refCounts, int depth);
    void removeUnusedLocals();
    bool propagateAndRemoveStatements();
    // Propagate statemtents; return true if an indirect call is converted
    // to direct
    bool propagateStatements(int memDepth, int toDepth = -1);
    int	 findMaxDepth();					// Find max memory nesting depth

    void toSSAform(int memDepth, StatementSet& rs);
    void fromSSAform();
    void insertAssignAfter(Statement* s, int tempNum, Exp* right);
    // Insert statement a after statement s
    void insertStatementAfter(Statement* s, Statement* a);
    void conTypeAnalysis(Prog* prog);
    void dfaTypeAnalysis(Prog* prog);
    void ellipsisTruncation();		// e.g. trim printf to appropriate number of parameters

    // For the final pass of removing returns that are never used
    typedef std::map<UserProc*, std::set<Exp*, lessExpStar> > ReturnCounter;
    void countUsedReturns(ReturnCounter& rc);
    bool removeUnusedReturns(ReturnCounter& rc);
    void doCountReturns(Statement* def, ReturnCounter& rc, Exp* loc);

    // Insert actual arguments to match formals
    void insertArguments(StatementSet& rs);

    // returns true if the prover is working right now
    bool canProveNow();
    // prove any arbitary property of this procedure
    bool prove(Exp *query);
    // helper function, should be private
    bool prover(Exp *query, std::set<PhiAssign*> &lastPhis,
                std::map<PhiAssign*, Exp*> &cache, PhiAssign *lastPhi = NULL);

    // promote the signature if possible
    void promoteSignature();

    // get all the statements
    void getStatements(StatementList &stmts);

    virtual void removeReturn(Exp *e);
    virtual void addReturn(Exp *e);

    // remove a statement
    void removeStatement(Statement *stmt);

    // inline constants / decode function pointer constants
    void processConstants();

    // get internal statements
    // Note: assignment causes shallow copy of list
    //virtual void getInternalStatements(StatementList &sl) {sl = internal;}
    // Calculate uses info
    void computeUses();

    bool searchAll(Exp* search, std::list<Exp*> &result);

#if 0
    // get the set of locations "defined" in this procedure
    void getDefinitions(LocationSet &defs) {
        defs = definesSet;
    }

    // get the set of locations "returned" by this procedure
    void getReturnSet(LocationSet &ret) {
        ret = returnsSet;
    }

#endif

    void getDefinitions(LocationSet &defs);

    virtual Memo *makeMemo(int mId);
    virtual void readMemo(Memo *m, bool dec);

private:
    /*
     * Find a pointer to the Exp* representing the given var
     * Used by the above 2
     */
    Exp** findVarEntry(int idx);

    /*
     * A special pass to check the sizes of memory that is about to be converted
     * into a var, ensuring that the largest size used in the proc is used for
     * all references (and it's declared that size)
     */
    void	checkMemSizes();

    /*
     * Implement the above for one given Exp*
     */
    void	checkMemSize(Exp* e);

    /*
     * Return an expression that is equivilent to e in terms of local variables.
     * Creates new locals as needed.
     */
    Exp *getLocalExp(Exp *le, Type *ty = NULL, bool lastPass = false);

public:

    /*
     * Sets the parameters that have been recovered for this procedure through
     * analysis.
     */
    void setParams(std::list<TypedExp*>& params, bool aggUsed = false);

    /*
     * Given a machine dependent location, return a generated symbolic
     * representation for it.
     */
    void toSymbolic(TypedExp* loc, TypedExp* result, bool local = true);

    /*
     * Return the next available local variable; make it the given type
     * Note: was returning TypedExp*
     */
    Exp* newLocal(Type* ty);

    // return a local's type
    Type *getLocalType(const char *nam);
    void setLocalType(const char *nam, Type *ty);

    // return a local's exp
    Exp *getLocalExp(const char *nam);
    void setLocalExp(const char *nam, Exp *e);

    int getNumLocals() {
        return (int)locals.size();
    }
    const char *getLocalName(int n) {
        int i = 0;
        for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++, i++)
            if (i == n)
                return (*it).first.c_str();
        return NULL;
    }
    void renameLocal(const char *oldName, const char *newName);
    virtual void renameParam(const char *oldName, const char *newName);

    /*
     * Add new locals, local<b> to local<n-1>
     */
    void addLocals(int b, int n);

    /*
     * Print the locals declaration in C style.
     */
    void printLocalsAsC(std::ostream& os);

    /*
     * Return the index of the first symbolic local for the procedure.
     */
    int getFirstLocalIndex();

    /*
     * Return the index of the last symbolic local for the procedure.
     */
    int getLastLocalIndex();

    /*
     * Return the list of symbolic locals for the procedure.
     */
    std::vector<TypedExp*>& getSymbolicLocals();

    /*
     * Replace each instance of a location in this procedure with its symbolic
     * representation if it has one.
     */
    void propagateSymbolics();

    /*
     * Get the BB that is the entry point (not always the first BB)
     */
    PBB getEntryBB();

    /*
     * Prints this procedure to an output stream.
     */
    std::ostream& put(std::ostream& os);

    /*
     * Set the entry BB for this procedure (constructor has the entry address)
     */
    void setEntryBB();

    /*
     * Get the callees
     */
    std::set<Proc*>& getCallees() {
        return calleeSet;
    }

    /*
     * Add to the set of callees
     */
    void addCallee(Proc* callee);

    /*
     * Add to a set of callee Procs
     */
    void addCallees(std::set<UserProc*>& callees);

    /*
     * return true if this procedure contains the given address
     */
    bool containsAddr(ADDRESS uAddr);

    /*
     * Change BB containing this statement from a COMPCALL to a CALL
     */
    void undoComputedBB(Statement* stmt) {
        cfg->undoComputedBB(stmt);
    }

    /*
     * Return true if this proc uses the special aggregate pointer as the
     * first parameter
     */
    virtual bool isAggregateUsed() {
        return aggregateUsed;
    }

    virtual Exp* getProven(Exp* left);

    virtual void printCallGraphXML(std::ostream &os, int depth,
                                   bool recurse = true);
    void printDecodedXML();
    void printAnalysedXML();
    void printSSAXML();
    void printXML();
    void printUseGraph();


    bool searchAndReplace(Exp *search, Exp *replace);

    // Visitation

    // Strip the refs from each expression
    void stripRefs();
    // Cast the constant whose conscript is num to be type ty
    void castConst(int num, Type* ty);

private:
    // We ensure that there is only one return statement now. See code in
    // frontend/frontend.cpp handling case STMT_RET.
    // If no return statement, this will be NULL
    ReturnStatement* theReturnStatement;
public:
    ADDRESS getTheReturnAddr() {
        return theReturnStatement == NULL ? NO_ADDRESS :
               theReturnStatement->getRetAddr();
    }
    void setTheReturnAddr(ReturnStatement* s, ADDRESS r) {
        assert(theReturnStatement == NULL);
        theReturnStatement = s;
        theReturnStatement->setRetAddr(r);
    }
    ReturnStatement* getTheReturnStatement() {
        return theReturnStatement;
    }
protected:
    friend class XMLProgParser;
    UserProc() : Proc(), cfg(NULL), decoded(false), analysed(false), decompileSeen(false), decompiled(false), isRecursive(false) { }
    void setCFG(Cfg *c) {
        cfg = c;
    }
    void addDef(Exp *e) {
        definesSet.insert(e);
    }
};		// class UserProc
#endif
