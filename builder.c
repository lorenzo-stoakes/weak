#include <strings.h>
#include "weak.h"

#define INIT_SIZE 10

static void expand(StringBuilder*);

void
AppendString(StringBuilder *builder, char *str)
{
  int strLen = strlen(str);

  if(strLen <= 0) {
    return;
  }

  if(builder->cap < builder->len) {
    panic("Impossible.");
  }
  if(builder->cap == builder->len) {
    expand(builder);
  }

  builder->strings[builder->len] = strdup(str);
  builder->len++;

  builder->Length += strLen;
}

char*
BuildString(StringBuilder *builder, bool releaseBuilder)
{
  char *ret;
  int i, j, k;

  char *str;

  if(builder->Length <= 0) {
    return NULL;
  }

  ret = (char*)allocate(sizeof(char), builder->Length + 1);

  k = 0;
  for(i = 0; i < builder->len && k < builder->Length; i++) {
    str = builder->strings[i];
    for(j = 0; str[j] != '\0'; j++) {
      ret[k] = str[j];
      k++;
    }
  }

  ret[k] = '\0';

  if(releaseBuilder) {
    ReleaseStringBuilder(builder);
  }

  return ret;
}

StringBuilder
NewStringBuilder()
{
  StringBuilder ret;

  ret.Length = 0;

  ret.cap = INIT_SIZE;
  ret.len = 0;
  ret.strings = (char**)allocate(sizeof(char*), INIT_SIZE);

  return ret;
}

void
ReleaseStringBuilder(StringBuilder *builder)
{
  int i;

  for(i = 0; i < builder->len; i++) {
    release(builder->strings[i]);
  }
}

static void
expand(StringBuilder *builder)
{
  char **buffer;

  builder->cap *= 2;
  buffer = (char**)allocate(sizeof(char*), builder->cap);
  memcpy(buffer, builder->strings, builder->len*sizeof(char*));
  release(builder->strings);
  builder->strings = buffer;
}
