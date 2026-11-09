#include "PreProcessor.hpp"

namespace Game
{
  PC_FreeDefine_t PC_FreeDefine = PC_FreeDefine_t(0x4E0D60);
  PC_FindHashedDefine_t PC_FindHashedDefine = PC_FindHashedDefine_t(0x421E00);
  PC_ReadToken_t PC_ReadToken = PC_ReadToken_t(0x4ACCD0);
  PC_ReadTokenHandle_t PC_ReadTokenHandle = PC_ReadTokenHandle_t(0x4D2060);
  PC_ReadSourceToken_t PC_ReadSourceToken = PC_ReadSourceToken_t(0x4B16F0);
  PC_UnreadSourceToken_t PC_UnreadSourceToken = PC_UnreadSourceToken_t(0x47CD00);
  PC_SourceError_t PC_SourceError = PC_SourceError_t(0x467A00);

  SourceError_t SourceError = SourceError_t(0x44C6C0);

  PC_Directive_if_def_t PC_Directive_if_def = PC_Directive_if_def_t(0x490A70);

  PC_Directive_if_t PC_Directive_if = PC_Directive_if_t(0x486220);
  PC_Directive_ifdef_t PC_Directive_ifdef = PC_Directive_ifdef_t(0x4F4ED0);
  PC_Directive_ifndef_t PC_Directive_ifndef = PC_Directive_ifndef_t(0x42EF10);
  PC_Directive_elif_t PC_Directive_elif = PC_Directive_elif_t(0x41AAB0);
  PC_Directive_else_t PC_Directive_else = PC_Directive_else_t(0x4B55B0);
  PC_Directive_endif_t PC_Directive_endif = PC_Directive_endif_t(0x491920);
  PC_Directive_include_t PC_Directive_include = PC_Directive_include_t(0x495310);
  PC_Directive_define_t PC_Directive_define = PC_Directive_define_t(0x42E460);
  PC_Directive_undef_t PC_Directive_undef = PC_Directive_undef_t(0x4E1820);
  PC_Directive_line_t PC_Directive_line = PC_Directive_line_t(0x4FD8A0);
  PC_Directive_error_t PC_Directive_error = PC_Directive_error_t(0x494AA0);
  PC_Directive_pragma_t PC_Directive_pragma = PC_Directive_pragma_t(0x42C160);
  PC_Directive_eval_t PC_Directive_eval = PC_Directive_eval_t(0x57DB20);
  PC_Directive_evalfloat_t PC_Directive_evalfloat = PC_Directive_evalfloat_t(0x4BC2A0);
}
