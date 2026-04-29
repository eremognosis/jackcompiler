# Error Code Catalog

All runtime errors are emitted as structured JSON logs on `stderr` with these standard fields:
`timestamp`, `severity`, `trace_id`, `code`, `component`, `message`, and `location` (`file`, `line`, `function`).

Use `trace_id` to correlate a specific error event across logs and source location.

| Code | Severity | Component | Trigger | Meaning | Typical action |
| --- | --- | --- | --- | --- | --- |
| `CC-SYM-001` | ERROR | `symtab` | `malloc` fails in symbol table allocation paths | Symbol table memory allocation failed | Check memory pressure and allocation limits |
| `CC-SYM-002` | ERROR | `symtab` | API called with `SymbolTable* == NULL` | Symbol table operation invoked without initialized table | Fix call ordering and table initialization |
| `CC-SYM-003` | ERROR | `symtab` | `symtab_define` receives invalid input (`name/type == NULL` or kind invalid) | Invalid symbol definition request | Validate parser/compiler inputs before define |
| `CC-VMW-001` | ERROR | `vmwriter` | Invalid `Segment` enum value | VM writer received unsupported segment | Verify segment mapping before emit |
| `CC-VMW-002` | ERROR | `vmwriter` | `FILE* out == NULL` in VM writer APIs | VM writer has no output stream | Ensure output file is opened and passed through |
| `CC-VMW-003` | ERROR | `vmwriter` | `vm_write_pop` with `SEG_CONST` | Illegal pop target (`constant`) | Fix codegen to pop only writable segments |
| `CC-VMW-004` | ERROR | `vmwriter` | Invalid `Command` enum value | VM writer received unsupported arithmetic command | Validate command generation logic |
| `CC-VMW-005` | ERROR | `vmwriter` | label argument is `NULL` | Branch/label emission missing label name | Ensure label generation always returns non-null |
| `CC-VMW-006` | ERROR | `vmwriter` | function name argument is `NULL` | Function/call emission missing function name | Ensure function symbol/name resolution succeeds |
