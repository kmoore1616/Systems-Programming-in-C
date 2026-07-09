import pandas as pd
import nltk
from nltk.corpus import words

# Download word list if not already downloaded
nltk.download('words')

# Load English words into a set for fast lookup
english_words = set(words.words())

def count_words_in_string(s):
    """Counts how many valid words are found within the given string."""
    count = 0
    s = s.lower()
    
    # Generate all possible substrings and check if they are valid words
    for i in range(len(s)):
        for j in range(i + 1, len(s) + 1):
            if s[i:j] in english_words:
                count += 1
    return count

def process_csv(file_path):
    """Reads the CSV file, extracts the second column, and counts valid words."""
    # Read the CSV file, assuming no headers
    df = pd.read_csv(file_path, header=None, usecols=[1], names=['string'], dtype=str)
    
    # Apply the word counting function
    df['word_count'] = df['string'].apply(count_words_in_string)

    # Get the top 10 strings with the most words
    top_strings = df.sort_values(by='word_count', ascending=False).head(100)

    return top_strings[['string', 'word_count']].values.tolist()

# Example usage:
file_path = './brute_force_results.csv'  # Replace with the actual file path
top_ten = process_csv(file_path)

# Print the results
for string, count in top_ten:
    print(f'"{string}" contains {count} valid words.')

