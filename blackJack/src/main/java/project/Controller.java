package project;

/**
 * Name: 
 * Username: 
 */

import javafx.fxml.FXML;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;

public class Controller {

	@FXML
    private Label result;

    @FXML
    private HBox phbox;

    @FXML
    private HBox dhbox;

    @FXML
    private Label phvalue;

    @FXML
    private Label dhvalue;

    @FXML
    private Label pscore;

    @FXML
    private Label dscore;

    @FXML
    private Button hit;

    @FXML
    private Button stand;
    
    @FXML
    private Button play;

    private Deck deck;
    private Player player;
    private Player dealer;

    @FXML
    void initialize() {
    	hit.setVisible(false);
    	stand.setVisible(false);
    	
        deck = new Deck();
        player = new Player();
        dealer = new Player();
        updateDisplay();
    }

    @FXML
    void hit() {
        Card card = deck.dealCard();
        player.addCardToHand(card);
        updateDisplay();

        if (player.isBusted()) {
            endGame();
        }
    }

    @FXML
    void stand() {
        // Dealer's turn
        while (dealer.getValueOfHand() < 17) {
            Card card = deck.dealCard();
            dealer.addCardToHand(card);
        }
        updateDisplay();
        endGame();
    }

    @FXML
    void startGame() {
    	
    	hit.setVisible(true);
    	stand.setVisible(true);
    	play.setVisible(false);
    	result.setText("RESULT!!!");
    	
        deck.reset();
        player.clearHand();
        dealer.clearHand();

        // Deal initial cards
        player.addCardToHand(deck.dealCard());
        dealer.addCardToHand(deck.dealCard());
        player.addCardToHand(deck.dealCard());
        dealer.addCardToHand(deck.dealCard());

        updateDisplay();

        // Check for player blackjack
        if (player.getValueOfHand() == 21) {
            stand();
        }
    }

    private void updateDisplay() {
        phbox.getChildren().clear();
        for (Card card : player.getHand()) {
            phbox.getChildren().add(card.getImageView());
            phbox.setAlignment(Pos.BOTTOM_CENTER);
        }
        phvalue.setText(String.valueOf(player.getValueOfHand()));

        dhbox.getChildren().clear();
        for (Card card : dealer.getHand()) {
            dhbox.getChildren().add(card.getImageView());
            dhbox.setAlignment(Pos.BOTTOM_CENTER);
        }
        dhvalue.setText(String.valueOf(dealer.getValueOfHand()));
    }

    private void endGame() {
        hit.setVisible(false);
        stand.setVisible(false);
        play.setVisible(true);

        // Determine winner
        int playerValue = player.getValueOfHand();
        int dealerValue = dealer.getValueOfHand();

        if (player.isBusted() || (!dealer.isBusted() && dealerValue > playerValue)) {
            dscore.setText(String.valueOf(dealer.getWins()));
            result.setText("Dealer WINS!!!");
        } else if (dealer.isBusted() || playerValue > dealerValue) {
            pscore.setText(String.valueOf(player.getWins()));
            result.setText("Player WINS!!!");
        }
        else {
        	result.setText("TIE NO ONE WINS!!!");
        }

        // Display result
        if (player.isBusted()) {
            phvalue.setText("Bust!");
        }
        if (dealer.isBusted()) {
            dhvalue.setText("Bust!");
        }
      }
}
