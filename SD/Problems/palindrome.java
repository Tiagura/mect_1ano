import java.util.Scanner;

//create java class with name palindrome
public class palindrome {
    //create main method
    public static void main(String[] args) {
        //create scanner object
        Scanner input = new Scanner(System.in);
        //ask user for 2 words
        System.out.println("Enter word: ");
        //create string variables
        String word = input.next();

        //create palindrome_checker object
        palindrome_checker palindrome = new palindrome_checker(word);
        //check if words are palindrome
        if (palindrome.is_palindrome())
            System.out.println("Word is a palindrome");
        else
            System.out.println("Word isn't a palindrome");
        
    }
}

//create class with name fifo
class fifo {
    //add atrributes
    private String word;

    //create constructor
    public fifo(String word) {
        //check if word is empty
        if (word.isEmpty()){
            System.out.println("Word is empty");
        }
        this.word = word;
    }
    
    public char unqueue_char(){
        //check if word is empty
        if (word.isEmpty()){
            System.out.println("Word is empty");
            return 0;
        }
        char letter = word.charAt(0);
        this.word = word.substring(1);
        return letter;
    }

    //size method
    public int size(){
        return word.length();
    }

}

class palindrome_checker{
    private fifo word, palindrome;

    public palindrome_checker(String word){
        //create fifo objects
        this.word = new fifo(word);
        this.palindrome = new fifo(reverse(word));
    }

    //reverse method
    private String reverse(String word){
        String reverse = "";
        for (int i = word.length() - 1; i >= 0; i--){
            reverse += word.charAt(i);
        }
        return reverse;
    }

    public boolean is_palindrome(){
        //check if words are empty
        if (word.size() == 0 || palindrome.size() == 0){
            return false;
        }

        //check if words are palindrome
        while (word.size() > 0){
            if (Character.toLowerCase(word.unqueue_char()) != Character.toLowerCase(palindrome.unqueue_char())){
                return false;
            }
        }
        return true;
    }
}