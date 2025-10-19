// Enes Yildirim 211ADB121
// https://github.com/Enesyildirim2/RTU_Programming_Languages_C_Lab_Fall_2025
// Simple arithmetic evaluator
// Compile gcc -O2 -Wall -Wextra -std=c17 -o bin/calc src/calc.c -lm
// I used Ai to help me in some parts of the commenting and code structuring
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  Data structures

// Defines the possible types for a numerical value in the expression
// This is important for handling mixed arithmetic operations
typedef enum { TYPE_INT, TYPE_DOUBLE } value_type_t;

// Shows a numeric value, which can be either an integer or a double.
// Uses a union to save memory, as only one type is active at a time.
typedef struct {
  value_type_t type;  // stores the actual type of the value
  union {
    long long i_val;  // for integer values
    double d_val;     // for double values
  } as;
} value_t;

// token kinds
typedef enum {
  TOKEN_NUMBER,  // A numeric literal
  TOKEN_ADD,     // + operator
  TOKEN_MINUS,   // - operator
  TOKEN_MULT,    // * operator
  TOKEN_DIV,     // / operator
  TOKEN_EOF,     // end of file/input stream
  TOKEN_ERROR    // invalid token

} token_type_t;

// Represents a single token extracted from the input string.
// Contains its type, actual value, position for error reporting,
// and a text representation for debugging.
typedef struct {
  token_type_t type;  // The kind of token.
  value_t value;      // The numerical value if type is TOKEN_NUMBER.
  long pos;  // 1-based position in the original input string where this token
             // starts. Useful for precise error reporting to the user.
  const char* text;  // Pointer to the textual representation of the token in
                     // the input string. Used for debugging.
} token_t;

// Stores the current state of the parser and tokenizer.
// This structure is passed around to all parsing functions, maintaining
// context about the input, current position, and any encountered errors.
typedef struct {
  const char* input;  // Pointer to the full input string (e.g., file contents).
  long input_len;     // Total length of the input string.
  long current_pos;   // 0-based index into 'input' indicating the current
                      // processing position.
  token_t current_token;  // The token recently retrieved by the tokenizer,
  int has_error;   // A flag indicating if an error has occurred during parsing.
                   // Set to 1 upon the first error.
  long error_pos;  // 1-based character position of the first error detected.
                   // Used for reporting error location to the user.
} parser_state_t;

// forward declarations
// This is necessary because parse_expression might be called by other parsing
// functions, or for good practice if the functions
// are defined later in the file. In this specific grammar, parse_expression
// calls parse_term which in turn calls parse_number.
static value_t parse_expression(parser_state_t* state);

// Helpers
// Records an error in the parser state.
// It ensures that only the first error encountered is recorded, preserving
// the location of the initial problem, which is often more useful for
// debugging.
static void set_error(parser_state_t* state, long pos) {
  if (!state->has_error) {   // Only record if no error has been set yet.
    state->has_error = 1;    // Set the error flag.
    state->error_pos = pos;  // Record the 1-based position of the error.
  }
}

// Converts an integer value_t to a double value_t in-place if it's currently an
// integer. Handling type promotion in mixed-type arithmetic. If the value is
// already a double, it returns it unchanged.
static value_t promote_to_double(value_t v) {
  if (v.type == TYPE_INT) {           // Check if the value is an integer.
    v.as.d_val = (double)v.as.i_val;  // Convert the integer part to a double.
    v.type = TYPE_DOUBLE;             // Update the type to double.
  }
  return v;  // Return the value.
}

// Prints a value_t to the file stream
// - Integers are printed as long long.
// - Doubles that are effectively integral are also printed as long long.
// - Other doubles are printed with 15 digits of precision using '%g'.
static void print_value(FILE* stream, value_t v) {
  if (v.type == TYPE_INT) {
    fprintf(stream, "%lld\n", v.as.i_val);  // Print integer directly.
    return;
  }

  // If the value is a double, check if it's effectively integral.
  // This means if its fractional part is very close to zero.
  // The threshold 1e-12 accounts for potential floating-point inaccuracies.
  long long rounded =
      (v.as.d_val > 0.0)
          ? (long long)(v.as.d_val + 0.5)   // Round for positive numbers
          : (long long)(v.as.d_val - 0.5);  // Round for negative numbers
  // Compare the original double with its rounded long long equivalent.
  if (fabs(v.as.d_val - (double)rounded) < 1e-12) {
    fprintf(stream, "%lld\n",
            rounded);  // Print as integer if effectively integral.
  } else {
    fprintf(
        stream, "%.15g\n",
        v.as.d_val);  // Otherwise, print as a double with general precision.
  }
}

//  Tokenizer

// Advances the current_pos in the input string past any whitespace characters.
// This ensures that the tokenizer always starts parsing from non-whitespace
// characters.
static void skip_whitespace(parser_state_t* state) {
  // Loop while the current position is within bounds and the character is
  // whitespace. Unsigned char is used with isspace() for safety with non-ASCII
  // or negative char values.
  while (state->current_pos < state->input_len &&
         isspace((unsigned char)state->input[state->current_pos])) {
    state->current_pos++;  // Move to the next character.
  }
}
// Retrieves the next token from the input string based on the parser's current
// position. It handles numbers, operators, EOF, and errors.
static token_t get_next_token(parser_state_t* state) {
  skip_whitespace(state);  // First, skip any leading whitespace.

  token_t tok;           // Declare a token structure to populate.
  tok.type = TOKEN_EOF;  // Initialize with EOF as a default.
  tok.pos = state->current_pos +
            1;    // Record the 1-based start position of the token.
  tok.text = "";  // Default empty text for now.

  // Check for End Of File/Input.
  if (state->current_pos >= state->input_len) {
    tok.type = TOKEN_EOF;  // Set token type to EOF.
    tok.text = "EOF";      // Set textual representation.
    return tok;            // Return the EOF token.
  }

  char c = state->input[state->current_pos];  // Get the current character.
  tok.pos = state->current_pos + 1;  // Update token's 1-based position.

  // Handle Numbers (integers or floating)
  // A number can start with a digit or a decimal point followed by a digit.
  if (isdigit((unsigned char)c) ||
      (c == '.' && state->current_pos + 1 < state->input_len &&
       isdigit((unsigned char)state->input[state->current_pos + 1]))) {
    char* endptr =
        NULL;  // Pointer used by strtod to indicate where parsing stopped.
    errno =
        0;  // Clear errno before strtod to reliably detect conversion errors.

    // strtod converts a string to a double. It handles integers, floats, and
    // scientific notation.
    double d = strtod(&state->input[state->current_pos], &endptr);
    // Check for conversion errors:
    // - If endptr is the same as the start pointer, no digits were found.
    // - If errno is ERANGE, the number was out of the representable range for
    // double.
    if (endptr == &state->input[state->current_pos] || errno == ERANGE) {
      set_error(state, tok.pos);  // Record an error at the token's position.
      tok.type = TOKEN_ERROR;     // Mark token as an error.
      tok.text = "number";        // Descriptive text for the error.
      return tok;
    }

    // Determine if the literal itself contained a decimal point or exponent.
    // This is important for preserving type intention.
    int is_float = 0;
    for (const char* p = &state->input[state->current_pos]; p < endptr; ++p) {
      if (*p == '.' || *p == 'e' || *p == 'E') {
        is_float = 1;  // If any float indicator is found, treat as double.
        break;
      }
    }

    tok.type = TOKEN_NUMBER;  // Set token type.
    if (is_float) {           // If a float indicator was present.
      tok.value.type = TYPE_DOUBLE;
      tok.value.as.d_val = d;  // Store the double value.
    } else {
      tok.value.type = TYPE_INT;
      tok.value.as.i_val =
          (long long)d;  // Store as long long, assuming it fits.
    }

    // Update the parser's current position to past the end of the number
    // literal.
    state->current_pos = (long)(endptr - state->input);
    return tok;
  }

  // --- Handle Operators ---
  // Simple switch for single-character operators.
  switch (c) {
    case '+':
      tok.type = TOKEN_ADD;
      tok.text = "+";
      state->current_pos++;  // Consume the character.
      break;
    case '-':
      tok.type = TOKEN_MINUS;
      tok.text = "-";
      state->current_pos++;
      break;
    case '*':
      tok.type = TOKEN_MULT;
      tok.text = "*";
      state->current_pos++;
      break;
    case '/':
      tok.type = TOKEN_DIV;
      tok.text = "/";
      state->current_pos++;
      break;
    default:                      // Handle unrecognized characters as errors.
      set_error(state, tok.pos);  // Record the error position.
      tok.type = TOKEN_ERROR;     // Mark as error token.
      tok.text = "invalid";
      state->current_pos++;  // Advance to avoid infinite loop on bad character.
      break;
  }

  return tok;  // Return the identified token.
}

// A helper function for the parser to consume an expected token.
// If the current token matches the expected_type, it advances to the next
// token. Otherwise, it signals a parsing error.
static void eat(parser_state_t* state, token_type_t type) {
  if (state->current_token.type ==
      type) {  // Check if the current token is the expected type.
    state->current_token =
        get_next_token(state);  // If yes, get the next token.
  } else {
    // If not, a syntax error occurred.
    set_error(state, state->current_token.pos);  // Record the error position.
  }
}

// Parser (recursive descent)
// This section implements a recursive descent parser, following the grammar:
// expression := term { ("+"|"-") term }
// term       := number { ("*"|"/") number }
// number     := <NUMBER_TOKEN>
// This structure correctly handles operator precedence and left-associativity.

// Parses a single number literal.
// It expects the current token to be a TOKEN_NUMBER.
static value_t parse_number(parser_state_t* state) {
  // Define a default zero value to return in case of an error.
  value_t zero = {.type = TYPE_INT, .as.i_val = 0};
  if (state->has_error)
    return zero;  // If an error already exists, propagate it.

  token_t t = state->current_token;  // Get the current token.
  if (t.type == TOKEN_NUMBER) {      // If it's a number token.
    eat(state, TOKEN_NUMBER);        // Consume the number token.
    return t.value;                  // Return the value.
  }

  // If it's not a number token, it's a syntax error.
  set_error(state, t.pos);  // Record the error position.
  return zero;              // Return zero as a placeholder value.
}

// Parses a term, which involves multiplication and division operations.
// It handles terms like 5 * 2.
// Multiplication and division have higher precedence than addition and
// subtraction.
static value_t parse_term(parser_state_t* state) {
  // A term always starts with a number.
  value_t result = parse_number(state);
  // Loop to handle subsequent multiplication or division operators
  // (left-associativity).
  while (!state->has_error && (state->current_token.type == TOKEN_MULT ||
                               state->current_token.type == TOKEN_DIV)) {
    token_t op = state->current_token;    // Get the operator token.
    eat(state, op.type);                  // Consume the operator.
    value_t right = parse_number(state);  // Parse the right-hand operand.

    if (state->has_error)
      break;  // If an error occurred parsing the right operand, stop.

    // Type Promotion Logic: If either operand is a double, promote both to
    // double to ensure floating-point arithmetic is used for the operation.
    if (result.type == TYPE_DOUBLE || right.type == TYPE_DOUBLE) {
      result = promote_to_double(result);
      right = promote_to_double(right);
    }

    // Perform the arithmetic operation based on the operator.
    if (op.type == TOKEN_MULT) {
      if (result.type == TYPE_INT)
        result.as.i_val *= right.as.i_val;  // Integer multiplication.
      else
        result.as.d_val *= right.as.d_val;  // Double multiplication.
    } else {                                // TOKEN_DIV
      // For division, the divisor is explicitly converted to double for safety
      // and to check for division by zero using floating-point comparison.
      double divisor =
          (right.type == TYPE_INT) ? (double)right.as.i_val : right.as.d_val;
      if (divisor == 0.0) {        // Check for division by zero.
        set_error(state, op.pos);  // Record a division-by-zero error.
        break;                     // Stop processing this term.
      }
      result =
          promote_to_double(result);  // Division always results in a double.
      result.as.d_val /= divisor;     // Perform double division.
    }
  }
  return result;  // Return the calculated value of the term.
}

// Parses an "expression", which involves addition and subtraction operations.
// It handles expressions like 1 + 2, 5 - 3.
// Addition and subtraction have lower precedence than multiplication/division.
static value_t parse_expression(parser_state_t* state) {
  // An expression starts with a term. This implicitly handles precedence.
  value_t result = parse_term(state);
  // Loop to handle subsequent addition or subtraction operators.
  while (!state->has_error && (state->current_token.type == TOKEN_ADD ||
                               state->current_token.type == TOKEN_MINUS)) {
    token_t op = state->current_token;  // Get the operator token.
    eat(state, op.type);                // Consume the operator.
    value_t right = parse_term(state);  // Parse the right-hand operand .

    if (state->has_error)
      break;  // If an error occurred parsing the right operand, stop.

    // Type Promotion Logic: Same as for terms, if any operand is double,
    // promote both.
    if (result.type == TYPE_DOUBLE || right.type == TYPE_DOUBLE) {
      result = promote_to_double(result);
      right = promote_to_double(right);
    }

    // Perform the arithmetic operation based on the operator.
    if (op.type == TOKEN_ADD) {
      if (result.type == TYPE_INT)
        result.as.i_val += right.as.i_val;  // Integer addition.
      else
        result.as.d_val += right.as.d_val;  // Double addition.
    } else {                                // TOKEN_MINUS
      if (result.type == TYPE_INT)
        result.as.i_val -= right.as.i_val;  // Integer subtraction.
      else
        result.as.d_val -= right.as.d_val;  // Double subtraction.
    }
  }
  return result;  // Return the calculated value of the term.
}

// File reading

// Reads the entire content of a file into a dynamically allocated string.
// Returns a pointer to the string on success, or NULL on failure.
// The caller is responsible for freeing the returned string.
static char* read_file_contents(const char* path) {
  FILE* f = fopen(path, "rb");  // Open file in binary read mode ('rb')
  if (!f) return NULL;          // Return NULL if file cannot be opened.

  // Seek to the end of the file to determine its size.
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);  // Close file on error.
    return NULL;
  }
  long len = ftell(f);  // Get current position, which is the file size.
  if (len < 0) {        // ftell can return -1 on error.
    fclose(f);
    return NULL;
  }
  fseek(f, 0, SEEK_SET);  // Seek back to the beginning of the file.

  // Allocate memory for the file content plus a null terminator.
  char* buf = (char*)malloc((size_t)len + 1);
  if (!buf) {
    fclose(f);  // Close file on memory allocation failure.
    return NULL;
  }
  // Read the entire file content into the buffer.
  if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
    free(buf);  // Free buffer on read error.
    fclose(f);
    return NULL;
  }
  buf[len] =
      '\0';    // Null-terminate the string, essential for C string functions.
  fclose(f);   // Close the file.
  return buf;  // Return the dynamically allocated string.
}

// Main program entry point.
int main(int argc, char* argv[]) {
  // Check command-line arguments: exactly one argument is expected.
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file.txt>\n", argv[0]);
    return 1;  // Return non-zero for error.
  }

  const char* input_path = argv[1];  // Get the input file path from arguments.
  char* content = read_file_contents(input_path);  // Read file content.
  if (!content) {
    perror("read file");  // Print system error message if file reading failed.
    return 1;
  }

  // Initialize the parser state with the file content.
  parser_state_t state = {
      .input = content,
      .input_len =
          (long)strlen(content),  // Calculate length of the read content.
      .current_pos = 0,           // Start at the beginning of the input.
      .has_error = 0,             // No errors initially.
      .error_pos = 0};

  // Initialize the first token. This primes the parser for the
  // parse_expression call.
  state.current_token = get_next_token(&state);
  // Start the main parsing process.
  value_t result = parse_expression(&state);

  // After parsing the expression, check for any remaining non-whitespace
  // characters. This indicates an incomplete or malformed expression.
  if (!state.has_error) {     // Only check if no error has occurred yet.
    skip_whitespace(&state);  // Skip any trailing whitespace.
    if (state.current_pos < state.input_len) {
      // If there are still characters left, it's an error.
      set_error(&state,
                state.current_pos +
                    1);  // Mark error at the unexpected character's position.
    }
  }

  // Output file name is always 'Enes_Yildirim_211ADB121.txt'
  FILE* out = fopen("Enes_Yildirim_211ADB121.txt","w");  // Open the output file in write mode.
  if (!out) {
    perror("fopen");  // Report system error if file cannot be opened.
    free(content);
    return 1;
  }

  // --- Output Result ---
  if (state.has_error) {
    fprintf(out, "ERROR:%ld\n",
            state.error_pos);  // If error, print error message with position.
  } else {
    print_value(out, result);  // Otherwise, print the calculated value.
  }

  fclose(out);    // Close the output file.
  free(content);  // Free the memory allocated for file content.
  return 0;       // Indicate successful execution.
}