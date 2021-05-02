CXXFLAGS=-std=c++17

src = \
	core/game.cpp \
	core/player.cpp \
	core/role.cpp \
	core/role_ref.cpp \
	core/rulebook.cpp \
	core/wildcard.cpp \
	interface/console.cpp \
	interface/game_log.cpp \
	interface/game_screens.cpp \
	interface/help_screens.cpp \
	interface/names.cpp \
	interface/questions.cpp \
	interface/screen.cpp \
	interface/setup_screen.cpp \
	interface/text/format.cpp \
	interface/text/preprocess.cpp \
	cli/main.cpp \

objs = $(addprefix build/,$(src:.cpp=.o))


mafia: $(objs)
	$(CXX) $(CXXFLAGS) $(objs) -o mafia

$(objs): build/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@


clean:
	rm -r ./build
	rm ./mafia


.PHONY: clean
