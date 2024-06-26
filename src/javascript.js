// for testing purposes
const { Schema, model } = require("mongoose");

const gamesSchema = new Schema({
    "id": Number,
    "title": String,
    "thumbnail": String,
    "short_description": String,
    "game_url": String,
    "genre": String,
    "platform": String,
    "publisher": String,
    "developer": String,
    "release_date": Date,
    "freetogame_profile_url": String
});

const gamesModel = model("game", gamesSchema, "games");

module.exports.getGames = () => gamesModel.find();