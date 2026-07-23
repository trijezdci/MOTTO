## Abstract Syntax Tree ##

M2C encodes valid Modula-2 input in memory as an abstract syntax tree (AST).

The AST uses three kinds of nodes.

* an `EMPTY` sentinel node
* non-terminal nodes
* terminal nodes


#### The Sentinel Node ####

The sentinel node [`EMPTY`](#empty-node),
is used to encode the absence of an optional sub-node in non-terminal nodes.


#### Non-Terminal Nodes ####

Non-terminal nodes are used to encode non-terminal symbols such as modules, imports, definitions, declarations, statements and expressions.

[`AST`](#root-node),
[`DEFMOD`](#definition-module-node),
[`IMPLIST`](#import-list-node),
[`IMPORT`](#qualified-import-node),
[`UNQIMP`](#unqualified-import-node),
[`DEFLIST`](#definition-list-node),
[`CONSTDEF`](#constant-definition-node),
[`TYPEDEF`](#type-definition-node),
[`PROCDEF`](#procedure-definition-node),
[`SUBR`](#subrange-type-definition-node),
[`ENUM`](#enumeration-type-definition-node),
[`SET`](#set-type-definition-node),
[`ARRAY`](#array-type-definition-node),
[`RECORD`](#simple-record-type-definition-node),
[`EXTREC`](#extensible-record-type-definition-node),
[`VRNTREC`](#variant-record-type-definition-node),
[`POINTER`](pointer-type-definition-node),
[`PROCTYPE`](#procedure-type-definition-node),
[`INDEXLIST`](#array-index-type-list-node),
[`FIELDLISTSEQ`](#simple-field-list-sequence-node),
[`FIELDLIST`](#simple-field-list-node),
[`VFLISTSEQ`](#variant-field-list-sequence-node),
[`VFLIST`](#variant-field-list-node),
[`VARIANTLIST`](#variant-list-node),
[`VARIANT`](#variant-node),
[`CLABELLIST`](#case-label-list-node),
[`CLABELS`](#case-labels-node),
[`FTYPELIST`](#formal-type-list-node),
[`OPENARRAY`](#open-array-parameter-node),
[`CONSTP`](#const-parameter-node),
[`VARP`](#var-parameter-node),
[`FPARAMLIST`](#formal-parameter-list-node),
[`FPARAMS`](#formal-parameters-node),
[`IMPMOD`](#program-or-implementation-module-node),
[`BLOCK`](#block-node),
[`DECLLIST`](#declaration-list-node),
[`TYPEDECL`](#type-declaration-node),
[`VARDECL`](#variable-declaration-node),
[`PROC`](#procedure-declaration-node),
[`MODDECL`](#module-declaration-node),
[`VSR`](#variable-size-record-type-declaration-node),
[`VSFIELD`](#variable-size-field-node),
[`EXPORT`](#unqualified-export-node),
[`QUALEXP`](#qualified-export-node),
[`STMTSEQ`](#statement-sequence-node),
[`ASSIGN`](#assignment-node),
[`PCALL`](#procedure-call-node),
[`RETURN`](#return-statement-node),
[`WITH`](#with-statement-node),
[`IF`](#if-statement-node),
[`SWITCH`](#case-statement-node),
[`LOOP`](#loop-statement-node),
[`WHILE`](#while-statement-node),
[`REPEAT`](#repeat-statement-node),
[`FORTO`](#for-statement-node),
[`EXIT`](#exit-statement-node),
[`ARGS`](#actual-parameters-node),
[`ELSIFSEQ`](#elsif-sequence-node),
[`ELSIF`](#elsif-node),
[`CASELIST`](#case-list-node),
[`CASE`](#case-branch-node),
[`ELEMLIST`](#set-element-list-node),
[`RANGE`](#expression-range-node),
[`FIELD`](#record-field-selector-node),
[`INDEX`](#array-index-node),
[`DESIG`](#designator-node),
[`DEREF`](#pointer-dereference-node),
[`NOT`](#logical-negation-expression-node),
[`AND`](#logical-conjunction-expression-node),
[`OR`](#logical-disjunction-expression-node),
[`EQ`](#equality-expression-node),
[`NEQ`](#inequality-expression-node),
[`LT`](#less-than-expression-node),
[`LTEQ`](#less-than-or-equal-expression-node),
[`GT`](#greater-than-expression-node),
[`GTEQ`](#greater-than-or-equal-expression-node),
[`IN`](#set-membership-expression-node),
[`NEG`](#arithmetic-negation-expression-node),
[`PLUS`](#plus-expression-node),
[`MINUS`](#minus-expression-node),
[`STAR`](#star-expression-node),
[`SLASH`](#slash-expression-node),
[`DIV`](#euclidean-division-expression-node),
[`MOD`](#remainder-of-euclidean-division-expression-node),
[`SETDIFF`](#set-difference-expression-node),
[`FCALL`](#function-call-expression-node),
[`SETVAL`](#set-value-expression-node).


#### Terminal Nodes ####

Terminal nodes are used to encode terminal symbols such as filenames, options, identifiers, integer literals, real number literals, character code and quoted literals.

[`FILENAME`](#filename-node),
[`OPTIONS`](#compiler-options-node),
[`IDENT`](#identifier-node),
[`IDENTLIST`](#identifier-list-node),
[`QUALIDENT`](#qualified-identifier-node),
[`INTVAL`](#whole-number-value-node),
[`REALVAL`](#real-number-value-node),
[`CHRVAL`](#character-code-value-node),
[`QUOTEDVAL`](#quoted-character-or-string-value-node).


### Graphical Representation ###

AST nodes are represented graphically as follows:

#### Non-Terminal Nodes ####

Nodes are represented by rectangular boxes. Arrows indicate subnode connections.

![node-legend](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/node-legend.png)

#### Terminal Nodes ####

The subnodes of terminal nodes are filled in grey.

![termnode-legend](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/termnode-legend.png)

#### Labels ####

Labels are represented by rounded boxes.

![label-legend](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/label-legend.png)

#### Alternatives ####

Labels with multiple connections represent alternatives.

![altlabel-legend](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/altlabel-legend.png)


### Serialised Representation ###

Any AST node may be represented in a serialised format of the form

    ( nodetype subnode-0 subnode-1 subnode-2 ... subnode-N )

where the actual number of sub-nodes is dependent on the node type.

This form of tree representation is called an S-expression.


## AST Node Reference ##

The structure of the common AST is described in detail below.


### Empty Node ###

The `EMPTY` node encodes the absence of an optional sub-node.

#### Graph ####
![empty](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/empty.png)

#### S-Expression ####

```
emptyNode :=
  '(' EMPTY ')'
  ;
```


### Root Node ###

The `AST` node encodes the **root** of the syntax tree.

#### Graph ####
![root](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/root.png)

#### S-Expression ####
```
astRootNode :=
  '(' AST filename options compilationUnit ')'
  ;

filename := filenameNode ; /* terminal node */

options := optionsNode ; /* terminal node */

compilationUnit :=
  defModuleNode | impModuleNode
  ;
```


### Definition Module Node ###

The `DEFMOD` node encodes a definition module.

#### Graph ####
![defmod](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/defmod.png)

#### S-Expression ####
```
defModuleNode :=
  '(' DEFMOD moduleIdent importList definitionList ')'
  ;

moduleIdent := identNode ; /* terminal node */

importList :=
  importListNode | emptyNode
  ;

definitionList :=
  definitionListNode | emptyNode
  ;
```


### Import List Node ###

The `IMPLIST` node encodes the entirety of import directives in a module.

#### Graph ####
![implist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/implist.png)

#### S-Expression ####
```
importListNode :=
  '(' IMPLIST import+ ')'
  ;
```


### Import Nodes ###

There are two types of import nodes:

[`IMPORT`](#qualified-import-node),
[`UNQIMP`](#unqualified-import-node).

![imp-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/imp-label.png)

```
import :=
  importNode | unqImportNode
  ;
```


### Qualified Import Node ###

The `IMPORT` node encodes a qualified import directive.

#### Graph ####
![import](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/import.png)

#### S-Expression ####
```
importNode :=
  '(' IMPORT identList ')'
  ;
  
identList := identListNode ; /* terminal node */
```


### Unqualified Import Node ###

The `UNQIMP` node encodes an unqualified import directive.

#### Graph ####
![unqimp](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/unqimp.png)

#### S-Expression ####
```
unqImportNode :=
  '(' UNQIMP moduleIdent identList ')'
  ;
```


### Definition List Node ###

The `DEFLIST` node encodes one or more definitions.

#### Graph ####
![deflist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/deflist.png)

#### S-Expression ####
```
definitionListNode :=
  '(' DEFLIST definition+ ')'
  ;
```


### Definition Nodes ###

There are four types of definition nodes:

[`CONSTDEF`](#constant-definition-node),
[`TYPEDEF`](#type-definition-node),
[`VARDECL`](#variable-declaration-node),
[`PROCDEF`](#procedure-definition-node).

![definition-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/definition-label.png)

```
  
definition :=
  constDefNode | typeDefNode | varDeclNode | ProcDefNode
  ;
```


### Constant Definition Node ###

The `CONSTDEF` node encodes a constant definition.

#### Graph ####
![constdef](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/constdef.png)

#### S-Expression ####
```
constDefNode :=
  '(' CONSTDEF identNode exprNode ')'
  ;
```  


### Type Definition Node ###

The `TYPEDEF` node encodes a type definition.

#### Graph ####
![typedef](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/typedef.png)

#### S-Expression ####
```
typeDefNode :=
  '(' TYPEDEF identNode ( typeNode | emptyNode ) ')'
  ;
```


### Procedure Definition Node ###

The `PROCDEF` node encodes a procedure definition.

#### Graph ####
![procdef](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/procdef.png)

#### S-Expression ####
```
procDefNode :=
  '(' PROCDEF identNode formalParamList returnedType ')'
  ;

formalParamList :=
  formalParamListNode | emptyNode
  ;

returnedType := qualidentNode ; /* terminal node */
```  


### Type Nodes ###

There are eleven nodes that may represent the definition part of a type definition or an anonymous type. 

[`IDENT`](#identifier-node),
[`QUALIDENT`](#qualified-identifier-node),
[`SUBR`](#subrange-type-definition-node),
[`ENUM`](#enumeration-type-definition-node),
[`SET`](#set-type-definition-node),
[`ARRAY`](#array-type-definition-node),
[`RECORD`](#simple-record-type-definition-node),
[`EXTREC`](#extensible-record-type-definition-node),
[`VRNTREC`](#variant-record-type-definition-node),
[`POINTER`](#pointer-type-definition-node),
[`PROCTYPE`](#procedure-type-definition-node).

![type-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/type-label.png)

```
typeNode :=
  identNode | qualidentNode |
  subrTypeNode | enumTypeNode | setTypeNode | arrayTypeNode | recTypeNode |
  extRecTypeNode | vrntRecTypeNode | pointerTypeNode | procTypeNode
  ;
```

Nine nodes may represent the type definition part of a field definition or an array base type. 

[`IDENT`](#identifier-node),
[`QUALIDENT`](#qualified-identifier-node),
[`SUBR`](#subrange-type-definition-node),
[`ENUM`](#enumeration-type-definition-node),
[`SET`](#set-type-definition-node),
[`ARRAY`](#array-type-definition-node),
[`RECORD`](#simple-record-type-definition-node),
[`POINTER`](#pointer-type-definition-node),
[`PROCTYPE`](#procedure-type-definition-node).

![fieldtype-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fieldtype-label.png)

```
fieldType :=
  identNode | qualidentNode | subrTypeNode | enumTypeNode | setTypeNode |
  arrayTypeNode | recTypeNode | pointerTypeNode | procTypeNode
  ;
```

Three nodes may represent the type definition part of a derived type.

[`IDENT`](#identifier-node),
[`QUALIDENT`](#qualified-identifier-node),
[`SUBR`](#subrange-type-definition-node).

![derivedtype-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/derivedtype-label.png)

```
derivedType :=
  identNode | qualidentNode | subrTypeNode
  ;
```

Three nodes represent the type definition part of a record type.

[`RECORD`](#simple-record-type-definition-node),
[`EXTREC`](#extensible-record-type-definition-node),
[`VRNTREC`](#variant-record-type-definition-node).

![recordtype-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/recordtype-label.png)

```
recordType :=
  recTypeNode | extRecTypeNode | vrntRecTypeNode
  ;
```


### Subrange Type Definition Node ###

The `SUBR` node encodes a subrange type definition.

#### Graph ####
![subrange](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/subr.png)

#### S-Expression ####
```
subrTypeNode :=
  '(' SUBR lowerBound upperBound subrBaseType ')'
  ;

lowerBound := exprNode ;

upperBound := exprNode ;

subrBaseType :=
  qualidentNode | emptyNode
  ;
```


### Enumeration Type Definition Node ###

The `ENUM` node encodes an enumeration type definition.

#### Graph ####
![enumeration](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/enum.png)

#### S-Expression ####
```
enumTypeNode :=
  '(' ENUM identListNode ')'
  ;
```


### Set Type Definition Node ###

The `SET` node encodes a set type definition.

#### Graph ####
![set](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/set.png)

#### S-Expression ####
```
setTypeNode :=
  '(' SET countableType ')'
  ;

countableType :=
  qualidentNode | subrTypeNode | enumTypeNode
  ; 
```  


### Array Type Definition Node ###

The `ARRAY` node encodes an array type definition.

#### Graph ####
![array](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/array.png)

#### S-Expression ####
```
arrayTypeNode :=
  '(' ARRAY indexTypeListNode arrayBaseType ')'
  ;

arrayBaseType := fieldType ;
```


### Simple Record Type Definition Node ###

The `RECORD` node encodes a non-variant non-extensible record type definition.

#### Graph ####
![record](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/record.png)

#### S-Expression ####
```
recTypeNode :=
  '(' RECORD fieldListSeqNode ')'
  ;
```


### Pointer Type Definition Node ###

The `POINTER` node encodes a pointer type definition.

#### Graph ####
![pointer](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/pointer.png)

#### S-Expression ####
```
pointerTypeNode :=
  '(' POINTER typeNode ')'
  ;
```


### Procedure Type Definition Node ###

The `PROCTYPE` node encodes a procedure type definition.

#### Graph ####
![proctype](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/proctype.png)

#### S-Expression ####
```
procTypeNode :=
  '(' PROCTYPE formalTypeList returnedType ')'
  ;

formalTypeList :=
  formalTypeListNode | emptyNode
  ;

returnedType :=
   qualidentNode | emptyNode
   ;
```


### Extensible Record Type Definition Node ###

The `EXTREC` node encodes an extensible record type definition.

#### Graph ####
![extrec](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/extrec.png)

#### S-Expression ####
```
extRecTypeNode :=
  '(' EXTREC recBaseType fieldListSeqNode ')'
  ;

recBaseType := qualidentNode ; /* terminal node */
```


### Variant Record Type Definition Node ###

The `VRNTREC` node encodes a variant record type definition.

#### Graph ####
![vrntrec](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vrntrec.png)

#### S-Expression ####
```
variantRecTypeNode :=
  '(' VRNTREC variantFieldListSeqNode ')'
  ;
```


### Array Index Type List Node ###

The `INDEXLIST` node encodes one or more index types within an array type definition.

#### Graph ####
![indexlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/indexlist.png)

#### S-Expression ####
```
indexTypeListNode :=
  '(' INDEXLIST indexType+ ')'
  ;

indexType := countableType ;
```


### Simple FieldList Sequence Node ###

The `FIELDLISTSEQ` node encodes a non-variant field list sequence within a record type definition.

#### Graph ####
![fieldlistseq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fieldlistseq.png)

#### S-Expression ####
```
fieldListSeqNode :=
  '(' FIELDLISTSEQ fieldListNode+ ')'
  ;
```


### Simple FieldList Node ###

The `FIELDLIST` node encodes a non-variant field list within a field list sequence

#### Graph ####
![fieldlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fieldlist.png)

#### S-Expression ####
```
fieldListNode :=
  '(' FIELDLIST identListNode fieldType ')'
  ;
```


### Variant FieldList Sequence Node ###

The `VFLISTSEQ` node encodes a field list sequence within a variant record type definition.

#### Graph ####
![vflistseq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vflistseq.png)

#### S-Expression ####
```
variantFieldListSeqNode :=
  '(' VFLISTSEQ ( fieldListNode | variantFieldListNode )+ ')'
  ;
```


### Variant Field List Node ###

The `VFLIST` node encodes a variant field list within a variant record field list sequence.

#### Graph ####
![vflist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vflist.png)

#### S-Expression ####
```
variantFieldListNode :=
  '(' VFLIST caseIdent caseType variantList defaultFieldListSeq ')'
  ;

caseIdent :=
  identNode | emptyNode
  ;

caseType := qualidentNode ; /* terminal node */

defaultFieldListSeq :=
  fieldListSeqNode | emptyNode
  ;
```


### Variant List Node ###

The `VARIANTLIST` node encodes a variant list within a variant field list.

#### Graph ####
![variantlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/variantlist.png)

#### S-Expression ####
```
variantList :=
  '(' VARIANTLIST variantNode+ ')'
  ;
```


### Variant Node ###

The `VARIANT` node encodes a variant within a variant list.

#### Graph ####
![variant](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/variant.png)

#### S-Expression ####
```
variantNode :=
  '(' VARIANT caseLabelListNode fieldListSeqNode ')'
  ;
```


### Case Label List Node ###

The `CLABELLIST` node encodes a case label list within a variant record definition or case statement.

#### Graph ####
![clabellist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/clabellist.png)

#### S-Expression ####
```
caseLabelListNode :=
  '(' CLABELLIST caseLabelsNode+ ')
  ;
```


### Case Labels Node ###

The `CLABELS` node encodes start and end labels within a case label list.

#### Graph ####
![clabels](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/clabels.png)

#### S-Expression ####
```
caseLabelsNode :=
  '(' CLABELS startLabel endLabel ')'
  ;

startLabel := exprNode ;

endLabel :=
  exprNode | emptyNode
  ;
```


### Formal Type List Node ###

The `FTYPELIST` node encodes a formal type list within a procedure type definition.

#### Graph ####
![ftypelist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/ftypelist.png)

#### S-Expression ####
```
formalTypeListNode :
  '(' FTYPELIST formalType+ ')'
  ;

formalType :=
  simpleFormalType | attrFormalType
  ;

simpleFormalType :=
  typeIdent | openArrayTypeNode
  ;

attrFormalType :=
  constAttrFormalTypeNode | varAttrFormalTypeNode
  ;
```


### Open Array Parameter Node ###

The `OPENARRAY` node encodes an open array parameter within a formal type list.

#### Graph ####
![openarray](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/openarray.png)

#### S-Expression ####
```
openArrayTypeNode :=
  '(' OPENARRAY typeIdent ')'
  ;

typeIdent := qualidentNode ; /* terminal node */
```


### CONST Parameter Node ###

The `CONSTP` node encodes a `CONST` parameter within a formal type list.

#### Graph ####
![constp](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/constp.png)

#### S-Expression ####
```
constAttrFormalTypeNode :=
  '(' CONSTP simpleFormalType ')'
  ;
```


### VAR Parameter Node ###

The `VARP` node encodes a `VAR` parameter within a formal type list.

#### Graph ####
![varp](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/varp.png)

#### S-Expression ####
```
varAttrFormalTypeNode :=
  '(' VARP simpleFormalType ')'
  ;
```


### Formal Parameter List Node ###

The `FPARAMLIST` node encodes a formal parameter list within a procedure type definition or procedure signature.

#### Graph ####
![fparamlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fparamlist.png)

#### S-Expression ####
```
formalParamListNode :=
  '(' FPARAMLIST formalParamsNode+ ')'
  ;
```


### Formal Parameters Node ###

The `FPARAMS` node encodes formal parameters within a formal parameter list.

#### Graph ####
![fparams](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fparams.png)

#### S-Expression ####
```
formalParamsNode :=
  '(' FPARAMS identListNode formalTypeNode ')'
  ;
```


### Program Or Implementation Module Node ###

The `IMPMOD` node encodes an implementation module.

#### Graph ####
![impmod](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/impmod.png)

#### S-Expression ####
```
impModuleNode :=
  '(' IMPMOD moduleIdent importListNode blockNode ')'
  ;
```


### Block Node ###

The `BLOCK` node encodes a block within a module or procedure.

#### Graph ####
![block](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/block.png)

#### S-Expression ####
```
blockNode :=
  '(' BLOCK declarationList body ')'
  ;

declarationList :=
  declarationListNode | emptyNode
  ;

body :=
  statementSeqNode | emptyNode
  ;
```


### Declaration List Node ###

The `DECLLIST` node encodes one or more declarations within a block.

#### Graph ####
![decllist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/decllist.png)

#### S-Expression ####
```
declarationListNode :=
  '(' DECLLIST declarationNode+ ')'
  ;
```


### Declaration Nodes ###

There are five types of declaration nodes:

[`CONSTDEF`](#constant-definition-node),
[`TYPEDECL`](#type-declaration-node),
[`VARDECL`](#variable-declaration-node),
[`PROC`](#procedure-declaration-node),
[`MODDECL`](#module-declaration-node).

![declaration-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/declaration-label.png)

```
declarationNode :=
  constDefNode | typeDeclNode | varDeclNode | procDeclNode | modDeclNode
  ;
```


### Type Declaration Node ###

The `TYPEDECL` node encodes a type declaration.

#### Graph ####
![typedecl](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/typedecl.png)

#### S-Expression ####
```
typeDeclNode :=
  '(' TYPEDECL identNode ( typeNode | vsrTypeNode ) ')'
  ;
```


### Variable Declaration Node ###

The `VARDECL` node encodes a variable or field declaration.

#### Graph ####
![vardecl](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vardecl.png)

#### S-Expression ####
```
varDeclNode :=
  '(' VARDECL identListNode fieldType ')'
  ;
```


### Procedure Declaration Node ###

The `PROC` node encodes a procedure declaration.

#### Graph ####
![proc](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/proc.png)

#### S-Expression ####
```
procDeclNode :=
  '(' PROC identNode formalParamListNode returnedType blockNode ')'
  ;
```


### Module Declaration Node ###

The `MODDECL` node encodes a local module declaration.

#### Graph ####
![moddecl](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/moddecl.png)

#### S-Expression ####
```
modDeclNode :=
  '(' MODDECL moduleIdent importListNode exportList blockNode ')'
  ;

exportList :=
  unqualExportNode | qualExportNode | emptyNode
  ;
```


### Variable Size Record Type Declaration Node ###

The `VSR` node encodes a variable size record type declaration.

#### Graph ####
![vsr](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vsr.png)

#### S-Expression ####
```
vsrTypeNode :=
  '(' VSR fieldListSeqNode varSizeFieldNode ')'
  ;
```


### Variable Size Field Node ###

The `VSFIELD` node encodes the indeterminate field of a variable size record type.

#### Graph ####
![vsfield](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/vsfield.png)

#### S-Expression ####
```
varSizeFieldNode :=
  '(' VSFIELD varSizeField determinantField varSizeFieldType ')'
  ;

varSizeField := identNode ; /* terminal node */

determinantField := identNode ; /* terminal node */

varSizeFieldType := qualidentNode ; /* terminal node */
```


### Unqualified Export Node ###

The `EXPORT` node encodes an unqualified export directive within a local module declaration.

#### Graph ####
![export](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/export.png)

#### S-Expression ####
```
unqualExportNode :=
  '(' EXPORT identListNode ')'
  ;
```


### Qualified Export Node ###

The `QUALEXP` node encodes a qualified export directive within a local module declaration.

#### Graph ####
![qualexp](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/qualexp.png)

#### S-Expression ####
```
qualExportNode :=
  '(' QUALEXP identListNode ')'
  ;
```


### Statement Sequence Node ###

The `STMTSEQ` node encodes a statement sequence.

#### Graph ####
![stmtseq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/stmtseq.png)

#### S-Expression ####
```
statementSeqNode :=
  '(' STMTSEQ statementNode+ ')'
  ;
```


### Statement Nodes ###

There are eleven types of statement nodes:

[`ASSIGN`](#assignment-node),
[`PCALL`](#procedure-call-node),
[`RETURN`](#return-statement-node),
[`WITH`](#with-statement-node),
[`IF`](#if-statement-node),
[`SWITCH`](#case-statement-node),
[`LOOP`](#loop-statement-node),
[`WHILE`](#while-statement-node),
[`REPEAT`](#repeat-statement-node),
[`FORTO`](#for-statement-node),
[`EXIT`](#exit-statement-node).

![stmt-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/stmt-label.png)

```
statementNode :=
  assignmentNode | controlStmt | withStmtNode
  ;
```

Nine statement nodes represent control statements:

[`PCALL`](#procedure-call-node),
[`RETURN`](#return-statement-node),
[`IF`](#if-statement-node),
[`SWITCH`](#case-statement-node),
[`LOOP`](#loop-statement-node),
[`WHILE`](#while-statement-node),
[`REPEAT`](#repeat-statement-node),
[`FORTO`](#for-statement-node),
[`EXIT`](#exit-statement-node).

![ctrlstmt-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/ctrlstmt-label.png)

```
controlStmt :=
  procCallNode | returnStmtNode | ifStmtNode | caseStmtNode | loopCtrlStmt
  ;
```

Five statement nodes represent loop control statements:

[`LOOP`](#loop-statement-node),
[`WHILE`](#while-statement-node),
[`REPEAT`](#repeat-statement-node),
[`FORTO`](#for-statement-node),
[`EXIT`](#exit-statement-node).

![loopctrlstmt-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/loopctrlstmt-label.png)

```
loopCtrlStatement :=
  loopStmtNode | whileStmtNode | repeatStmtNode | ForToStmtNode | exitStmtNode
  ;
```


### Assignment Node ###

The `ASSIGN` node encodes an assignment statement.

#### Graph ####
![assign](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/assign.png)

#### S-Expression ####
```
assignmentNode :=
  '(' ASSIGN designator exprNode ')'
  ;

designator :=
  identNode | qualidentNode | derefNode | designatorNode
  ;
```


### Procedure Call Node ###

The `PCALL` node encodes a procedure call statement.

#### Graph ####
![pcall](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/pcall.png)

#### S-Expression ####
```
procCallNode :=
  '( PCALL designator actualParams ')'
  ;

actualParams :=
  actualParamsNode | emptyNode
  ;
```


### RETURN Statement Node ###

The `RETURN` node encodes a `RETURN` statement.

#### Graph ####
![return](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/return.png)

#### S-Expression ####
```
returnStmtNode :=
  '(' RETURN returnValue ')'
  ;

returnValue :=
  exprNode | emptyNode
  ;
```


### WITH Statement Node ###

The `WITH` node encodes a `WITH` statement.

#### Graph ####
![with](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/with.png)

#### S-Expression ####
```
withStmtNode :=
  '(' WITH designator statementSeqNode ')'
  ;
```


### IF Statement Node ###

The `IF` node encodes an `IF` statement.

#### Graph ####
![if](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/if.png)

#### S-Expression ####
```
ifStmtNode :=
  '(' IF exprNode ifBranch elsifSeq elseBranch ')'
  ;

ifBranch := statementSeqNode ;

elsifSeq :=
  elsifSeqNode | emptyNode
  ;

elseBranch :=
  statementSeqNode | emptyNode
  ;
```


### CASE Statement Node ###

The `SWITCH` node encodes a `CASE` statement.

#### Graph ####
![switch](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/switch.png)

#### S-Expression ####
```
caseStmtNode :=
  '(' SWITCH designator caseListNode elseBranch ')'
  ;
```


### LOOP Statement Node ###

The `LOOP` node encodes a `LOOP` statement.

#### Graph ####
![loop](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/loop.png)

#### S-Expression ####
```
loopStmtNode :=
  '(' LOOP statementSeqNode ')'
  ;
```


### WHILE Statement Node ###

The `WHILE` node encodes a `WHILE` statement.

#### Graph ####
![while](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/while.png)

#### S-Expression ####
```
whileStmtNode :=
  '(' WHILE exprNode statementSeqNode ')'
  ;
```


### REPEAT Statement Node ####

The `REPEAT` node encodes a `REPEAT` statement.

#### Graph ####
![repeat](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/repeat.png)

#### S-Expression ####
```
repeatStmtNode :=
  '(' REPEAT statementSeqNode exprNode ')'
  ;
```


### FOR Statement Node ###

The `FORTO` node encodes a `FOR` statement.

#### Graph ####
![forto](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/forto.png)

#### S-Expression ####
```
forToStmtNode :=
  '(' FORTO identNode startValue endValue stepValue statementSeqNode ')'
  ;

startValue : exprNode ;

endValue := expNode ;

stepValue :=
  exprNode | emptyNode
  ;
```


### EXIT Statement Node ###

The `EXIT` node encodes an `EXIT` statement.

#### Graph ####
![exit](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/exit.png)

#### S-Expression ####
```
exitStmtNode :=
  '(' EXIT ')'
  ;
```


### Actual Parameters Node ###

The `ARGS` node encodes actual parameters in a procedure or function call.

#### Graph ####
![args](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/args.png)

#### S-Expression ####
```
actualParamsNode :=
  '(' ARGS exprNode+ ')'
  ;
```


### ELSIFSEQ Node ###

The `ELSIFSEQ` node encodes an `ELSIF` sequence within an `IF` statement.

#### Graph ####
![elsifseq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/elsifseq.png)

#### S-Expression ####
```
elsifSeqNode :=
  '(' ELSIFSEQ elsifNode+ ')'
  ;
```


### ELSIF Node ###

The `ELSIF` node encodes a single `ELSIF` branch within an `IF` statement.

#### Graph ####
![elsif](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/elsif.png)

#### S-Expression ####
```
elsifNode :=
  '(' ELSIF exprNode statementSeqNode ')'
  ;
```


### CASE List Node ###

The `CASELIST` node encodes a case list within a `CASE` statement.

#### Graph ####
![caselist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/caselist.png)

#### S-Expression ####
```
caseListNode :=
  '(' CASELIST caseBranchNode+ ')'
  ;
```


### CASE Branch Node ###

The `CASE` node encodes a case branch within a `CASE` statement.

#### Graph ####
![case](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/case.png)

#### S-Expression ####
```
caseBranchNode :=
  '(' CASE caseLabelListNode statementSeqNode ')'
  ;
```


### Set Element List Node ###

The `ELEMLIST` node encodes the element list within a set value.

#### Graph ####
![elemlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/elemlist.png)

#### S-Expression ####
```
elementListNode :=
  '(' ELEMLIST element+ ')'
  ;

element :=
  expr | range
  ;
```


### Expression Range Node ###

The `RANGE` node encodes a value range.

#### Graph ####
![range](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/range.png)

#### S-Expression ####
```
range :=
  '(' RANGE lowerValue upperValue ')'
  ;

lowerValue := exprNode ;

upperValue := exprNode ;
```


### Record Field Selector Node ###

The `FIELD` node encodes a record field selector.

#### Graph ####
![field](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/field.png)

#### S-Expression ####
```
fieldSelectorNode :=
  '(' FIELD selector ')'
  ;

selector :=
  qualidentNode | designatorNode
  ;
```


### Array Index Node ###

The `INDEX` node encodes an array subscript selector.

#### Graph ####
![index](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/index.png)

#### S-Expression ####
```
arrayIndexNode :=
  '(' INDEX subscript+ ')'
  ;

subscript := exprNode ;
```


### Designator Node ###

The `DESIG` node encodes a designator.

#### Graph ####
![desig](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/desig.png)

#### S-Expression ####
```
designatorNode :=
  '(' DESIG head tail ')'
  ;

head :=
  qualidentNode | derefNode
  ;

tail :=
  fieldSelectorNode | arrayIndexNode | emptyNode
  ;
```


### Pointer Dereference Node ###

The `DEREF` node encodes a pointer dereference.

#### Graph ####
![deref](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/deref.png)

#### S-Expression ####
```
derefNode :=
  '(' DEREF pointer ')'
  ;

pointer :=
  qualident | derefNode | designatorNode
  ;
```


### Expression Nodes ###

There are 25 nodes that may represent expressions or sub-expressions:

[`NOT`](#logical-negation-expression-node),
[`AND`](#logical-conjunction-expression-node),
[`OR`](#logical-disjunction-expression-node),
[`EQ`](#equality-expression-node),
[`NEQ`](#inequality-expression-node),
[`LT`](#less-than-expression-node),
[`LTEQ`](#less-than-or-equal-expression-node),
[`GT`](#greater-than-expression-node),
[`GTEQ`](#greater-than-or-equal-expression-node),
[`IN`](#set-membership-expression-node),
[`NEG`](#arithmetic-negation-expression-node),
[`PLUS`](#plus-expression-node),
[`MINUS`](#minus-expression-node),
[`STAR`](#star-expression-node),
[`SLASH`](#slash-expression-node),
[`DIV`](#euclidean-division-expression-node),
[`MOD`](#remainder-of-euclidean-division-expression-node),
[`SETDIFF`](#set-difference-expression-node),
[`DESIG`](#designator-node),
[`FCALL`](#function-call-expression-node),
[`SETVAL`](#set-value-expression-node).
[`INTVAL`](#whole-number-value-node),
[`REALVAL`](#real-number-value-node),
[`CHRVAL`](#character-code-value-node),
[`QUOTEDVAL`](#quoted-character-or-string-value-node).

![expr-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/expr-label.png)

```
expr :=
  boolExpr | relationalExpr | arithmeticExpr |
  designator | funcCallNode | setValNode | literalValue
  ;
```


There are three boolean expression nodes:

[`NOT`](#logical-negation-expression-node),
[`AND`](#logical-conjunction-expression-node),
[`OR`](#logical-disjunction-expression-node).

![boolexpr-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/boolexpr-label.png)

```
boolExpr :=
  notNode | andNode | orNode
  ;
```

There are seven relational expression nodes:

[`EQ`](#equality-expression-node),
[`NEQ`](#inequality-expression-node),
[`LT`](#less-than-expression-node),
[`LTEQ`](#less-than-or-equal-expression-node),
[`GT`](#greater-than-expression-node),
[`GTEQ`](#greater-than-or-equal-expression-node),
[`IN`](#set-membership-expression-node).

![relexpr-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/relexpr-label.png)

```
relationalExpr :=
  eqNode | neqNode | ltNode | ltEqNode | gtNode | gtEqNode | inNode
  ;
```

There are eight arithmetic expression nodes:

[`NEG`](#arithmetic-negation-expression-node),
[`PLUS`](#plus-expression-node),
[`MINUS`](#minus-expression-node),
[`STAR`](#star-expression-node),
[`SLASH`](#slash-expression-node),
[`DIV`](#euclidean-division-expression-node),
[`MOD`](#remainder-of-euclidean-division-expression-node),
[`SETDIFF`](#set-difference-expression-node).

![arithmexpr-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/arithmexpr-label.png)

```
arithmeticExpr :=
  negNode | plusNode | minusNode | starNode | slashNode |
  divNode | modulusNode | setDiffNode
  ;
```

There are four literal value expression nodes:

[`INTVAL`](#whole-number-value-node),
[`REALVAL`](#real-number-value-node),
[`CHRVAL`](#character-code-value-node),
[`QUOTEDVAL`](#quoted-character-or-string-value-node).

![litval-label](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/litval-label.png)

```
literalValue :=
  intValNode | realValNode | chrValNode | quotedValNode
  ;
```


### Logical Negation Expression Node ###

The `NOT` node encodes an expression of the form `NOT expr`.

#### Graph ####
![not](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/not.png)

#### S-Expression ####
```
notNode :=
  '(' NOT right ')'
  ;
```


### Logical Conjunction Expression Node ###

The `AND` node encodes an expression of the form `expr1 AND expr2`.

#### Graph ####
![and](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/and.png)

#### S-Expression ####
```
andNode :=
  '(' AND left right ')'
  ;
```


### Logical Disjunction Expression Node ###

The `OR` node encodes an expression of the form `expr1 OR expr2`.

#### Graph ####
![or](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/or.png)

#### S-Expression ####
```
orNode :=
  '(' OR left right ')'
  ;
```


### Equality Expression Node ###

The `EQ` node encodes an expression of the form `expr1 = expr2`.

#### Graph ####
![eq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/eq.png)

#### S-Expression ####
```
eqNode :=
  '(' EQ left right ')'
  ;

left : exprNode ;
```


### Inequality Expression Node ###

The `NEQ` node encodes an expression of the form `expr1 # expr2`.

#### Graph ####
![neq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/neq.png)

#### S-Expression ####
```
neqNode :=
  '(' NEQ left right ')'
  ;
```


### Less-Than Expression Node ###

The `LT` node encodes an expression of the form `expr1 < expr2`.

#### Graph ####
![lt](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/lt.png)

#### S-Expression ####
```
ltNode :=
  '(' LT left right ')'
  ;
```


### Less-Than-Or-Equal Expression Node ###

The `LTEQ` node encodes an expression of the form `expr1 <= expr2`.

#### Graph ####
![lteq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/lteq.png)

#### S-Expression ####
```
ltEqNode :=
  '(' LTEQ left right ')'
  ;
```


### Greater-Than Expression Node ###

The `GT` node encodes an expression of the form `expr1 > expr2`.

#### Graph ####
![gt](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/gt.png)

#### S-Expression ####
```
gtNode :=
  '(' GT left right ')'
  ;
```


### Greater-Than-Or-Equal Expression Node ###

The `GTEQ` node encodes an expression of the form `expr1 >= expr2`.

#### Graph ####
![gteq](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/gteq.png)

#### S-Expression ####
```
gtEqNode :=
  '(' GTEQ left right ')'
  ;
```


### Set Membership Expression Node ###

The `IN` node encodes an expression of the form `expr1 IN expr2`.

#### Graph ####
![in](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/in.png)

#### S-Expression ####
```
inNode :=
  '(' IN left right ')'
  ;
```


### Arithmetic Negation Expression Node ###

The `NEG` node encodes an expression of the form `- expr`.

#### Graph ####
![neg](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/neg.png)

#### S-Expression ####
```
negNode :=
  '(' NEG right ')'
  ;

right : exprNode ;
```


### Plus Expression Node ###

The `PLUS` node encodes an expression of the form `expr1 + expr2`.

#### Graph ####
![plus](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/plus.png)

#### S-Expression ####
```
plusNode :=
  '(' PLUS left right ')'
  ;
```


### Minus Expression Node ###

The `MINUS` node encodes an expression of the form `expr1 - expr2`.

#### Graph ####
![minus](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/minus.png)

#### S-Expression ####
```
minusNode :=
  '(' MINUS left right ')'
  ;
```


### Star Expression Node ###

The `STAR` node encodes an expression of the form `expr1 * expr2`.

#### Graph ####
![star](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/star.png)

#### S-Expression ####
```
starNode :=
  '(' STAR left right ')'
  ;
```


### Slash Expression Node ###

The `SLASH` node encodes an expression of the form `expr1 / expr2`.

#### Graph ####
![slash](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/slash.png)

#### S-Expression ####
```
slashNode :=
  '(' SLASH left right ')'
  ;
```


### Euclidean Division Expression Node ###

The `DIV` node encodes an expression of the form `expr1 DIV expr2`.

#### Graph ####
![div](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/div.png)

#### S-Expression ####
```
divNode :=
  '(' DIV left right ')'
  ;
```


### Remainder of Euclidean Division Expression Node ###

The `MOD` node encodes an expression of the form `expr1 MOD expr2`.

#### Graph ####
![mod](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/mod.png)

#### S-Expression ####
```
modulusNode :=
  '(' MOD left right ')'
  ;
```


### Set Difference Expression Node ###

The `SETDIFF` node encodes an expression of the form `expr1 \ expr2`.

#### Graph ####
![setdiff](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/setdiff.png)

#### S-Expression ####
```
setDiffNode :=
  '(' SETDIFF left right ')'
  ;
```


### Function Call Expression Node ###

The `FCALL` node encodes a function call expression.

#### Graph ####
![fcall](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/fcall.png)

#### S-Expression ####
```
funcCallNode :=
  '( FCALL designator actualParams ')'
  ;
```


### Set Value Expression Node ###

The `SETVAL` node encodes a set value expression.

#### Graph ####
![setval](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/setval.png)

#### S-Expression ####
```
setValNode :=
  '( SETVAL elementList setTypeIdent ')'
  ;

elementList :=
  actualParams | emptyNode
  ;

setTypeIdent :=
  ident | qualident | emptyNode
  ;
```


### Filename Node ###

The `FILENAME ` node encodes the filename of the source file.

#### Graph ####
![filename](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/filename.png)

#### S-Expression ####
```
filenameNode :=
  '(' FILENAME '"' filename '"' ')'
  ;
```

#### Example ####
```
(FILENAME "parser.mod")
```


### Compiler Options Node ###

The `OPTIONS` node encodes the compiler options used when compiling the source file.

#### Graph ####
![options](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/options.png)

#### S-Expression ####
```
optionsNode :=
  '(' OPTIONS ( '"' option-name '"' )+ ')'
  ;
```

#### Example ####
```
(OPTIONS "--pim4" "--no-synonyms" "--no-coroutines" ...)
```


### Identifier Node ###

The `IDENT` node encodes an identifier.

#### Graph ####
![ident](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/ident.png)

#### S-Expression ####
```
identNode :=
  '(' IDENT '"' Ident '"' ')'
  ;
```

#### Example ####
```
(IDENT "foobar")
```


### Identifier List Node ###

The `IDENTLIST` node encodes an identifier list.

#### Graph ####
![identlist](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/identlist.png)

#### S-Expression ####
```
identListNode :=
  '(' IDENTLIST ( '"' Ident '"' )+ ')'
  ;
```

#### Example ####
```
(IDENTLIST "foo" "bar" "baz" ...)
```


### Qualified Identifier Node ###

The `QUALIDENT` node encodes the component identifiers of a qualified identifier.

#### Graph ####
![qualident](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/qualident.png)

#### S-Expression ####
```
qualidentNode :=
  '(' QUALIDENT ( '"' Ident '"' ) ( '"' Ident '"' )+ ')'
  ;
```

#### Example ####
```
(QUALIDENT "foo" "bar" ...)
```


### Whole Number Value Node ###

The `INTVAL` node encodes a whole number value.

#### Graph ####
![intval](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/intval.png)

#### S-Expression ####
```
intValNode :=
  '(' INTVAL ( lexeme | '#' lexeme ) ')'
  ;
```

#### Examples ####
```
(INTVAL 12345)
(INTVAL #0x7FFF)
```


### Real Number Value Node ###

The `REALVAL` node encodes a real number value.

#### Graph ####
![realval](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/realval.png)

#### S-Expression ####
```
realValNode :=
  '(' REALVAL lexeme ')'
  ;
```

#### Examples ####
```
(REALVAL 1.234)
(REALVAL 5.678e9)
```


### Character Code Value Node ###

The `CHRVAL` node encodes a character code value.

#### Graph ####
![chrval](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/chrval.png)

#### S-Expression ####
```
chrValNode :=
  '(' CHRVAL '#' lexeme ')'
  ;
```

#### Example ####
```
(CHRVAL #0u7F)
```


### Quoted Character or String Value Node ###

The `QUOTEDVAL` node encodes a quoted character or string value.

#### Graph ####
![quotedval](https://github.com/m2sf/m2-ast-node-Diagrams/blob/master/png/quotedval.png)

#### S-Expression ####
```
quotedValNode :=
  '(' QUOTEDVAL '"' lexeme '"' ')'
  ;
```
  
#### Example ####
```
(QUOTEDVAL "quoted character or string")
```

+++
