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
 * OVERVIEW:   Interface for the procedure classes, which are used to store information about variables in the
 *				procedure such as parameters and locals.
 *============================================================================*/

/* $Revision: 1.115.2.24 $
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
#include "dataflow.h"			// For class UseCollector

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

virtual				~Proc();

		/*
		 * Gets name of this procedure.
		 */
		const char* getName();

		/*
		 * Gets sets the name of this procedure.
		 */
		void		setName(const char *nam);

		/*
		 * Get the native address.
		 */
		ADDRESS		getNativeAddress();

		/*
		 * Set the native address
		 */
		void		setNativeAddress(ADDRESS a);

		/*
		 * Get the program this procedure belongs to.
		 */
		Prog		*getProg() { return prog; }
		void		setProg(Prog *p) { prog = p; }
	 
		/*
		 * Get/Set the first procedure that calls this procedure (or null for main/start).
		 */
		Proc		*getFirstCaller();
		void		setFirstCaller(Proc *p) { if (m_firstCaller == NULL) m_firstCaller = p; }

		/*
		 * Returns a pointer to the Signature
		 */
		Signature	*getSignature() { return signature; }
		void		setSignature(Signature *sig) { signature = sig; }

virtual void		renameParam(const char *oldName, const char *newName);

		/*
		 * Prints this procedure to an output stream.
		 */
//virtual std::ostream& put(std::ostream& os) = 0;

		/*
		 * Modify actuals so that it is now the list of locations that must
		 * be passed to this procedure. The modification will be to either add
		 * dummy locations to actuals, delete from actuals, or leave it
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
		void		matchParams(std::list<Exp*>& actuals, UserProc& caller);

		/*
		 * Get a list of types to cast a given list of actual parameters to
		 */
		std::list<Type>* Proc::getParamTypeList(const std::list<Exp*>& actuals);

		/*
		 * Return true if this is a library proc
		 */
virtual bool		isLib() {return false;}

		/*
		 * Return true if the aggregate pointer is used.
		 * It is assumed that this is false for library procs
		 */
virtual bool		isAggregateUsed() {return false;}

		/*
		 * OutPut operator for a Proc object.
		 */
		friend std::ostream& operator<<(std::ostream& os, Proc& proc);
		
virtual Exp			*getProven(Exp *left) = 0;
virtual	bool		isPreserved(Exp* e) = 0;		// Return whether e is preserved by this proc

		// Set an equation as proven. Useful for some sorts of testing
		void		setProven(Exp* fact) {proven.insert(fact);}

		/*
		 * Get the callers
		 * Note: the callers will be in a random order (determined by memory allocation)
		 */
		std::set<CallStatement*>& getCallers() { return callerSet; }

		/*
		 * Add to the set of callers
		 */
		void		addCaller(CallStatement* caller) { callerSet.insert(caller); }

		/*
		 * Add to a set of caller Procs
		 */
		void		addCallers(std::set<UserProc*>& callers);

		void		removeParameter(Exp *e);
virtual	void		removeReturn(Exp *e);
//virtual void		addReturn(Exp *e);
//		void		sortParameters();

virtual void		printCallGraphXML(std::ostream &os, int depth, bool recurse = true);
		void		printDetailsXML();
		void		clearVisited() { visited = false; }
		bool		isVisited() { return visited; }

		Cluster		*getCluster() { return cluster; }
		void		setCluster(Cluster *c) { cluster = c; }

virtual Memo		*makeMemo(int mId) = 0;
virtual void		readMemo(Memo *m, bool dec) = 0;

protected:

		bool		visited;						// For printCallGraphXML

		Prog		*prog;							// Program containing this procedure.

		/*
		 * The formal signature of this procedure. This information is determined
		 * either by the common.hs file (for a library function) or by analysis.
		 *
		 * NOTE: This belongs in the CALL, because the same procedure can have different signatures if it happens to
		 * have varargs. Temporarily here till it can be permanently moved.
		 */
		Signature	*signature;

		/* Persistent state */
		ADDRESS		address;						// Procedure's address.
		Proc		*m_firstCaller;					// first procedure to call this procedure.
		ADDRESS		m_firstCallerAddr;				// can only be used once.
		// All the expressions that have been proven true. (Could perhaps do with a list of some that are proven false.)
		// Of the form r29 = r29 (NO subscripts) or r28 = r28 + 4
		std::set<Exp*, lessExpStar> proven;
		std::set<CallStatement*> callerSet;			// Set of callers (CallStatements that call this procedure).
		Cluster		*cluster;						// Cluster this procedure is contained within.

		friend class XMLProgParser;
		Proc() : visited(false), prog(NULL), signature(NULL), address(0), m_firstCaller(NULL), m_firstCallerAddr(0),
			cluster(NULL) { }

};	// class Proc

/*==============================================================================
 * LibProc class.
 *============================================================================*/
class LibProc : public Proc {
public:
	
					LibProc(Prog *prog, std::string& name, ADDRESS address);
virtual				~LibProc();

#if 0
		/*
		 * See comment for Proc::matchParams.
		 */
		void		matchParams(std::list<Exp*>& actuals, UserProc& caller, const Parameters* outgoing,
						const Exp** intRetLoc) const;
#endif

		/*
		 * Return true, since is a library proc
		 */
		bool		isLib() {return true;}

		/*
		 * Return true if the aggregate pointer is used.
		 * It is assumed that this is false for library procs
		 */
virtual bool		isAggregateUsed() {return false;}

virtual Exp*		getProven(Exp* left);
virtual	bool		isPreserved(Exp* e);			// Return whether e is preserved by this proc

		/*
		 * Prints this procedure to an output stream.
		 */
		//std::ostream& put(std::ostream& os);

virtual Memo		*makeMemo(int mId);
virtual void		readMemo(Memo *m, bool dec);

		void		getInternalStatements(StatementList &internal);
protected:

		friend class XMLProgParser;
					LibProc() : Proc() { }

};		// class LibProc

enum ProcStatus {
	PROC_UNDECODED,		// Has not even been decoded
	PROC_DECODED,		// Decoded, no attempt at decompiling
	PROC_SORTED,		// Decoded, and CFG has been sorted by address
	PROC_VISITED,		// Has been visited on the way down in decompile()
	PROC_INCYCLE,		// Is involved in cycles, has not completed initial decompilation as yet
	PROC_PREPRES,		// Has had enough propagation etc to perform preservation analysis
	PROC_PRESERVES,		// Has had preservation analysis done
	PROC_INITIAL,		// Has had initial decompiling only (some callees were involved in recursion)
	PROC_FINAL			// Has had final decompilation
	// , PROC_RETURNS	// Has had returns intersected with all caller's defines
};

typedef std::set <UserProc*> CycleSet;
typedef std::list<UserProc*> CycleList;

/*==============================================================================
 * UserProc class.
 *============================================================================*/

class UserProc : public Proc {

		/*
		 * The control flow graph.
		 */
		Cfg*		cfg;

		/**
		 * The status of this user procedure
		 */
		ProcStatus	status;			// Status: undecoded .. final decompiled

		/*
		 * True if this procedure has been decoded.
		 */
		//bool		decoded;		// Deprecated; use ProcStatus
		
		// true if the procedure has been analysed.
		//bool		analysed;		// Deprecated; use ProcStatus

		/*
		 * Indicate that the aggregate location pointer "hidden" parameter is used, and is thus explicit in this
		 * translation.  Needed only by architectures like Sparc where a special parent stack location is used to pass
		 * the address of aggregates. Set with the setParams() member function.
		 */
		//bool		aggregateUsed;

		/**
		 * Maximum depth this function (or limited by a command line switch). -1 if not set.
		 */
		int			maxDepth;

		/*
		 * This map records the names and types for local variables. It should be a subset of the symbolMap, which also
		 * stores parameters.
		 * It is a convenient place to store the types of locals after
		 * conversion from SSA form, since it is then difficult to access the definitions of locations.
		 * This map could be combined with symbolMap below, but beware of parameters (in symbols but not locals)
		 */
		std::map<std::string, Type*> locals;

		int			nextLocal;		// Number of the next local. Can't use locals.size() because some get deleted

		/*
		 * A map between machine dependent locations and their corresponding symbolic, machine independent
		 * representations.  Example: m[r28{0} - 8] -> local5
		 */
public:
		typedef std::map<Exp*,Exp*,lessExpStar> SymbolMapType;
private:
		SymbolMapType symbolMap;

		/*
		 * Set of callees (Procedures that this procedure calls). Used for call graph, among other things
		 */
		std::list<Proc*> calleeList;
	 
		/**
		 * A collector for initial parameters (locations used before being defined)
		 * Note that final parameters don't use this; it's only of use during group decompilation analysis (sorting out
		 * recursion)
		 */
		UseCollector col;

		/**
		 * The list of parameters, ordered and filtered.
		 * Note that a LocationList could be used, but then there would be nowhere to store the types (for DFA based TA)
		 * The RHS is just ignored; the list is of ImplicitAssigns
		 */
		StatementList parameters;

		// The modifieds for the procedure are now stored in the return statement

		/**
		 * DataFlow object. Holds information relevant to transforming to and from SSA form.
		 */
		DataFlow	df;

		/*
		 * Current statement number. Makes it easier to split initialiseDecompile from initialDecompile.
		 */
		int			stmtNumber;

		/*
		 * Pointer to a set of procedures involved in a recursion group.
		 * NOTE: Each procedure in the cycle points to the same set!
		 */
		CycleSet*	cycleGroup;

public:

					UserProc(Prog *prog, std::string& name, ADDRESS address);
virtual				~UserProc();

		/*
		 * Records that this procedure has been decoded.
		 */
		void		setDecoded();

		/*
		 * Removes the decoded bit and throws away all the current information about this procedure.
		 */
		void		unDecode();

		/*
		 * Returns a pointer to the CFG.
		 */
		Cfg*		getCFG() { return cfg; }

		/*
		 * Deletes the whole CFG and all the RTLs and Exps associated with it. Also nulls the internal cfg
		 * pointer (to prevent strange errors)
		 */
		void		deleteCFG();

		/**
		 * Lookup the expression in the symbol map. Return NULL or a C string with the symbol.
		 */
		char*		lookupSym(Exp* e);

		/**
		 * Determine whether e is a local, either as a true opLocal (e.g. generated by fromSSA), or if it is in the
		 * symbol map and the name is in the locals map. If it is a local, return its name, else NULL
		 */
		char*		findLocal(Exp* e);

		/*
		 * Returns an abstract syntax tree for the procedure in the internal representation. This function actually
		 * _calculates_ * this value and is expected to do so expensively.
		 */ 
		SyntaxNode	*getAST();
		// print it to a file
		void		printAST(SyntaxNode *a = NULL);

		/*
		 * Returns whether or not this procedure can be decoded (i.e. has it already been decoded).
		 */
		bool		isDecoded() { return status >= PROC_DECODED; }
		bool		isDecompiled() { return status >= PROC_FINAL; }

		bool		isSorted() { return status >= PROC_SORTED; }
		void		setSorted() { status = PROC_SORTED; }

		/*
		 * Return the number of bytes allocated for locals on the stack.
		 */
		int			getLocalsSize();

		// code generation
		void		generateCode(HLLCode *hll);

		// print this proc, mainly for debugging
		void		print(std::ostream &out);
		void		printParams(std::ostream &out);
		char		*prints();
		void		dump();
		void		printToLog();
		void		symbolMapToLog();			// Print just the symbol map
		void		dumpSymbolMap();			// For debugging
		void		dumpSymbols(std::ostream& os);
		void		dumpSymbols();
		void		dumpIgraph(igraph& ig);

		// simplify the statements in this proc
		void		simplify() { cfg->simplify(); }

		/// Begin the decompile process at this procedure
		CycleSet*	decompile(CycleList* path);
		/// Initialise decompile: sort CFG, number statements, dominator tree, etc
		void		initialiseDecompile();
		/// Prepare for preservation analysis only
		void		prePresDecompile();
		/// Initial decompile: to SSA, propagate, initial params and returns
		void		initialDecompile();
		/// Analyse the whole group of procedures for conditional preserveds, and update till no change
		/// Also finalise the whole group
		void		recursionGroupAnalysis(CycleSet* cycleSet);
		/// Remove unused statements
		void		removeUnusedStatements();
		/// Final decompile: final parameters, type analysis (unless ad-hoc), process constants
		void		finalDecompile();
		// Split the set of cycle-associated procs into individual subcycles
		//void		findSubCycles(CycleList& path, CycleSet& cs, CycleSetSet& sset);
		// The inductive preservation analysis.
		bool		inductivePreservation(UserProc* topOfCycle);
		// Mark calls involved in the recursion cycle as non childless (each child has had initialDecompile called on
		// it now)
		void		markAsNonChildless(CycleSet* cs);
		// Update the defines and arguments in calls
		void		updateCalls();

		void		propagateAtDepth(int depth);
		void		doRenameBlockVars(int depth, bool clearStacks);
		void		updateBlockVars();
		void		updateBlockVars(int minDepth);

		Statement	*getStmtAtLex(unsigned int begin, unsigned int end);

		// All the decompile stuff except propagation, DFA repair, and null/unused statement removal
		void    	complete(); 		// FIXME: is this used?

		// Initialise the statements, e.g. proc, bb pointers
		void		initStatements();
		void		numberStatements(int& stmtNum);
		void		numberPhiStatements(int& stmtNum);
		bool		nameStackLocations();
		bool		replaceReg(Exp* match, Exp* e, Statement* def);		// Helper function for nameRegisters()
		bool		nameRegisters();
		void		removeRedundantPhis();
		void		findPreserveds();			// Was trimReturns()
		void		findSpPreservation();		// Preservations only for the stack pointer
		void		updateReturnTypes();
		void		fixCallAndPhiRefs(int d);	// Perform call and phi statement bypassing at depth d
		void		fixCallAndPhiRefs();		// Perform call and phi statement bypassing at all depths
					// Helper function for the above
		void		fixRefs(int n, int depth, std::map<Exp*, Exp*, lessExpStar>& pres, StatementList& removes);
		void		initialParameters();		// Get initial parameters based on proc's use collector
		void		findFinalParameters();
		void		addParameter(Exp *e);		// Add to signature (temporary now; still needed to create param names)
		void		insertParameter(Exp* e);	// Insert into parameters list correctly sorted
//		void		addNewReturns(int depth);
		void		updateArguments();			// Update the arguments in calls
		void		updateCallDefines();		// Update the defines in calls
		// Trim parameters. If depth not given or == -1, perform at all depths
		void		trimParameters(int depth = -1);
		void		processFloatConstants();
		void		replaceExpressionsWithGlobals();
		void		replaceExpressionsWithSymbols();
		void		replaceExpressionsWithParameters(int depth);   // must be in SSA form
		void		mapExpressionsToLocals(bool lastPass = false);
		bool		isLocal(Exp* e);			// True if e represents a stack local variable
		bool		isLocalOrParam(Exp* e);		// True if e represents a stack local or stack param

		// find the procs the calls point to
		void		assignProcsToCalls();
		
		// perform final simplifications
		void		finalSimplify();
	
private:
		void		searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp, StatementList& stmts);
public:
		bool		removeNullStatements();
		bool		removeDeadStatements();
typedef std::map<Statement*, int> RefCounter;
		void		countRefs(RefCounter& refCounts);
		void		removeUnusedStatements(RefCounter& refCounts, int depth);
		void		removeUnusedLocals();
		bool		propagateAndRemoveStatements();
		// Propagate statemtents; return true if an indirect call is converted to direct
		bool		propagateStatements(int memDepth, int toDepth = -1);
		void		propagateToCollector(int depth);
		int			findMaxDepth();					// Find max memory nesting depth

		void		toSSAform(int memDepth, StatementSet& rs);
		void		fromSSAform();
		void		insertAssignAfter(Statement* s, Exp* left, Exp* right);
		// Insert statement a after statement s
		void		insertStatementAfter(Statement* s, Statement* a);
		void		conTypeAnalysis();
		void		dfaTypeAnalysis();
		// Trim parameters to procedure calls with ellipsis (...). Also add types for ellipsis parameters, if any
		// Returns true if any signature types so added
		bool		ellipsisProcessing();
		// Convert registers to locations (does not need multiple passes, or to call replaceExpressionsWithSymbols)
		void		mapRegistersToLocals();
		// This is a helper function for the above:
		void		regReplaceList(std::list<Exp**>& li);

		// For the final pass of removing returns that are never used
//typedef	std::map<UserProc*, std::set<Exp*, lessExpStar> > ReturnCounter;
		void		removeUnusedReturns(std::set<UserProc*>& removeRetSet);
		// Update parameters and call livenesses to take into account the changes causes by removing a return from this
		// procedure, or a callee's parameter (which affects this procedure's arguments, which are also uses).
		void		updateForUseChange(std::set<UserProc*>& removeRetSet);
		//void		 countUsedReturns(ReturnCounter& rc);
		//void		 doCountReturns(Statement* def, ReturnCounter& rc, Exp* loc);

		// returns true if the prover is working right now
		bool		canProveNow();
		// prove any arbitary property of this procedure
		bool		prove(Exp *query);
		// helper function, should be private
		bool		prover(Exp *query, std::set<PhiAssign*> &lastPhis, std::map<PhiAssign*, Exp*> &cache,
						Exp* original, PhiAssign *lastPhi = NULL);	  

		// promote the signature if possible
		void		promoteSignature();

		// get all the statements
		void		getStatements(StatementList &stmts);

virtual	void		removeReturn(Exp *e);
//virtual void		addReturn(Exp *e);

		// remove a statement
		void		removeStatement(Statement *stmt);

		// inline constants / decode function pointer constants
		bool		processConstants();

		// Calculate uses info
		void		computeUses();

		bool		searchAll(Exp* search, std::list<Exp*> &result);

		void		getDefinitions(LocationSet &defs);
		void		addImplicitAssigns();
		StatementList& getParameters() { return parameters; }
		StatementList& getModifieds() { return theReturnStatement->getModifieds(); }

virtual Memo		*makeMemo(int mId);
virtual void		readMemo(Memo *m, bool dec);

private:
		/*
		 * Find a pointer to the Exp* representing the given var
		 * Used by the above 2
		 */
		Exp**		findVarEntry(int idx);

		/*
		 * A special pass to check the sizes of memory that is about to be converted into a var, ensuring that the
		 * largest size used in the proc is used for all references (and it's declared that size)
		 */
		void		checkMemSizes();

		/*
		 * Implement the above for one given Exp*
		 */
		void		checkMemSize(Exp* e);

public:
		/* 
		 * Return an expression that is equivilent to e in terms of local variables.  Creates new locals as needed.
		 */
		Exp			*getSymbolExp(Exp *le, Type *ty = NULL, bool lastPass = false);


		/*
		 * Sets the parameters that have been recovered for this procedure through analysis.
		 * Not currently implemented or called. Probably needed to deal with the Sparc aggregate pointer.
		 */
		void		setParams(std::list<TypedExp*>& params, bool aggUsed = false);

		/*
		 * Given a machine dependent location, return a generated symbolic representation for it.
		 */
		void		toSymbolic(TypedExp* loc, TypedExp* result, bool local = true);

		/*
		 * Return the next available local variable; make it the given type. Note: was returning TypedExp*
		 */
		Exp*		newLocal(Type* ty);

		// return a local's type
		Type		*getLocalType(const char *nam);
		void		setLocalType(const char *nam, Type *ty);

		// return a symbol's exp (note: the original exp, like r24, not local1)
		Exp			*expFromSymbol(const char *nam);
		void		setExpSymbol(const char *nam, Exp *e, Type* ty);

		int			getNumLocals() { return (int)locals.size(); }
		const char	*getLocalName(int n);
		char		*getSymbolName(Exp* e);		// As above, but look for expression e
		void		renameLocal(const char *oldName, const char *newName);
virtual void		renameParam(const char *oldName, const char *newName);

		void		setParamType(const char* nam, Type* ty);

		/*
		 * Print the locals declaration in C style.
		 */
		void		printLocalsAsC(std::ostream& os);

		/*
		 * Get the BB that is the entry point (not always the first BB)
		 */
		PBB			getEntryBB();

		/*
		 * Prints this procedure to an output stream.
		 */
		//std::ostream& put(std::ostream& os);

		/*
		 * Set the entry BB for this procedure (constructor has the entry address)
		 */
		void		setEntryBB();

		/*
		 * Get the callees
		 */
		std::list<Proc*>& getCallees() { return calleeList; }

		/*
		 * Add to the set of callees
		 */
		void		addCallee(Proc* callee); 

		/*
		 * Add to a set of callee Procs
		 */
		void		addCallees(std::list<UserProc*>& callees);

		/*
		 * return true if this procedure contains the given address
		 */
		bool		containsAddr(ADDRESS uAddr);

		/*
		 * Change BB containing this statement from a COMPCALL to a CALL
		 */
		void		undoComputedBB(Statement* stmt) {
						cfg->undoComputedBB(stmt); }

		/*
		 * Return true if this proc uses the special aggregate pointer as the
		 * first parameter
		 */
//virtual bool		isAggregateUsed() {return aggregateUsed;}

virtual Exp*		getProven(Exp* left);
virtual	bool		isPreserved(Exp* e);			// Return whether e is preserved by this proc

virtual void		printCallGraphXML(std::ostream &os, int depth,
									   bool recurse = true);
		void		printDecodedXML();
		void		printAnalysedXML();
		void		printSSAXML();
		void		printXML();
		void		printUseGraph();


		bool		searchAndReplace(Exp *search, Exp *replace);

		// Cast the constant whose conscript is num to be type ty
		void		castConst(int num, Type* ty);

		// Add a location to the UseCollector; this means this location is used before defined, and hence is an
		// *initial* parameter. Note that final parameters don't use this information; it's only for handling recursion.
		void		useBeforeDefine(Exp* loc) {col.insert(loc);}
 
private:
		// We ensure that there is only one return statement now. See code in frontend/frontend.cpp handling case
		// STMT_RET.
		ReturnStatement* theReturnStatement;
public:
		ADDRESS		getTheReturnAddr() {
						return theReturnStatement == NULL ? NO_ADDRESS : theReturnStatement->getRetAddr();}
		void		setTheReturnAddr(ReturnStatement* s, ADDRESS r) {
						assert(theReturnStatement == NULL);
						theReturnStatement = s;
						theReturnStatement->setRetAddr(r);}
		ReturnStatement* getTheReturnStatement() {return theReturnStatement;}
		bool		filterReturns(Exp* e);			// Decide whether to filter out e (return true) or keep it
		bool		filterParams(Exp* e);			// As above but for parameters and arguments
protected:
		friend class XMLProgParser;
					UserProc();
		void		setCFG(Cfg *c) { cfg = c; }
};		// class UserProc
#endif
