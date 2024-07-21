package project;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;

public class Card {

    public static final String[] FACES = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    public static final String[] SUITS = {"hearts", "spades", "clubs", "diamonds"};
    public static final int HEIGHT = 130;

    private String face;
    private String suit;
    private ImageView cardImage;

    public Card(String face, String suit) {
        this.face = face;
        this.suit = suit;
        String imageName = face + "_of_" + suit + ".png"; // Adjusted image naming
        this.cardImage = new ImageView(new Image(imageName));
        this.cardImage.setFitHeight(HEIGHT);
        this.cardImage.setPreserveRatio(true);
    }

    public String getFace() {
        return face;
    }

    public String getSuit() {
        return suit;
    }

    public int getValue() {
        if (face.equals("ace")) {
            return 11; // Aces initially count as 11
        } else if (face.equals("J") || face.equals("Q") || face.equals("K")) {
            return 10;
        } else {
            return Integer.parseInt(face);
        }
    }

    public ImageView getImageView() {
        return cardImage;
    }
}
