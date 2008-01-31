

import tokenize as tokenize_module
import token_stream

tokenize = tokenize_module.tokenize
Token = tokenize_module.TokenInstance

def asTokenStream(source):
  """
  Convert 'source' into a TokenStream. Accepts a string, a list
  of tokens, or an existing TokenStream.

  If 'source' is already a TokenStream, this method returns the
  original object without modification.
  """

  if isinstance(source, token_stream.TokenStream):
    return source

  if isinstance(source, str):
    source = tokenize(source)
  
  return token_stream.TokenStream(source)

