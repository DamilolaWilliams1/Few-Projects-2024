package project;

import java.util.ArrayList;
import java.util.List;

public class Player {

    private List<Card> hand;
    private int wins;

    public Player() {
        hand = new ArrayList<>();
        wins = 0;
    }

    public List<Card> getHand() {
        return hand;
    }

    public int getValueOfHand() {
        int value = 0;
        boolean hasAce = false;
        for (Card card : hand) {
            value += card.getValue();
            if (card.getFace().equals("ace")) {
                hasAce = true;
            }
        }
        // Adjust for aces if total exceeds 21
        if (hasAce && value > 21) {
            value -= 10;
        }
        return value;
    }

    public void clearHand() {
        hand.clear();
    }

    public void addCardToHand(Card card) {
        hand.add(card);
    }

    public boolean isBusted() {
        return getValueOfHand() > 21;
    }

    public int getWins() {
        return wins= wins + 1;
    }

    public void incrementWins() {
        wins++;
    }
}
