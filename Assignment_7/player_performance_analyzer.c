#include "Players_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID 1000
#define MAX_TEAMS 10
#define NAME_LENGTH 50
#define PLAYERS_PER_TEAM 50

typedef enum
{
    INVALID_ROLE = 0,
    BATSMAN = 1,
    BOWLER = 2,
    ALL_ROUNDER = 3
} Role;

typedef struct PlayerNode
{
    int player_id;
    char name[NAME_LENGTH];
    char team_name[NAME_LENGTH];
    Role role;
    int total_runs;
    float batting_average;
    float strike_rate;
    int wickets;
    float economy_rate;
    float performance_index;
    struct PlayerNode *next;
} PlayerNode;

typedef struct Team
{
    int team_id;
    char name[NAME_LENGTH];
    int total_players;
    float avg_batting_strike_rate;
    PlayerNode *players;
    PlayerNode *batsmen_sorted;
    PlayerNode *bowlers_sorted;
    PlayerNode *all_rounders_sorted;
} Team;

typedef struct RolePlayerNode
{
    PlayerNode *player;
    struct RolePlayerNode *next;
} RolePlayerNode;

typedef struct
{
    RolePlayerNode *batsmen;
    RolePlayerNode *bowlers;
    RolePlayerNode *all_rounders;
} RoleBasedLists;

Team team_data[MAX_TEAMS];
RoleBasedLists role_lists;

void initializeData();
float calculatePerformanceIndex(Role, float, float, int, float);
void addPlayerToTeam();
void displayTeamPlayers();
void displayTeamsByStrikeRate();
void displayTopKPlayers();
void displayPlayersByRole();
int binarySearchTeam(int);
void insertPlayerSorted(RolePlayerNode **, PlayerNode *);
void insertPlayerInTeamRoleList(PlayerNode **, PlayerNode *);
void calculateTeamStats(Team *);
void displayMenu();

Role getRoleFromString(const char *);
const char *getRoleString(Role);
void cleanInputBuffer();
void freeMemory();

int main()
{
    role_lists.batsmen = NULL;
    role_lists.bowlers = NULL;
    role_lists.all_rounders = NULL;

    printf("Initializing ICC ODI Player Performance Analyzer...\n");
    initializeData();
    printf("Initialization complete! Loaded %d players from %d teams.\n", playerCount, teamCount);

    int choice;
    do
    {
        displayMenu();
        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input! Please enter a number.\n");
            cleanInputBuffer();
            continue;
        }

        switch (choice)
        {
        case 1:
            addPlayerToTeam();
            break;
        case 2:
            displayTeamPlayers();
            break;
        case 3:
            displayTeamsByStrikeRate();
            break;
        case 4:
            displayTopKPlayers();
            break;
        case 5:
            displayPlayersByRole();
            break;
        case 6:
            freeMemory();
            printf("\nExiting... Thank you!\n");
            break;
        default:
            printf("\nInvalid choice! Please try again.\n");
        }
    } while (choice != 6);

    return 0;
}

Role getRoleFromString(const char *roleStr)
{
    if (strcmp(roleStr, "Batsman") == 0)
    {
        return BATSMAN;
    }
    if (strcmp(roleStr, "Bowler") == 0)
    {
        return BOWLER;
    }
    if (strcmp(roleStr, "All-rounder") == 0)
    {
        return ALL_ROUNDER;
    }
    printf("Error: Invalid role '%s'. Valid roles are: Batsman, Bowler, All-rounder\n", roleStr);
    return INVALID_ROLE;
}

const char *getRoleString(Role role)
{
    switch (role)
    {
    case BATSMAN:
        return "Batsman";
    case BOWLER:
        return "Bowler";
    case ALL_ROUNDER:
        return "All-rounder";
    case INVALID_ROLE:
        return "Invalid";
    default:
        return "Unknown";
    }
}

float calculatePerformanceIndex(Role role, float battingAvg, float strikeRate, int wickets, float economyRate)
{
    switch (role)
    {
    case BATSMAN:
        return (battingAvg * strikeRate) / 100.0;
    case BOWLER:
        return (wickets * 2.0) + (100.0 - economyRate);
    case ALL_ROUNDER:
        return ((battingAvg * strikeRate) / 100.0) + (wickets * 2.0);
    case INVALID_ROLE:
        printf("Error: Cannot calculate performance index for invalid role\n");
        return 0.0;
    default:
        return 0.0;
    }
}

void insertPlayerInTeamRoleList(PlayerNode **head, PlayerNode *player)
{
    if (*head == NULL || (*head)->performance_index < player->performance_index)
    {
        player->next = *head;
        *head = player;
    }
    else
    {
        PlayerNode *players_list = *head;
        while (players_list->next != NULL && players_list->next->performance_index >= player->performance_index)
        {
            players_list = players_list->next;
        }
        player->next = players_list->next;
        players_list->next = player;
    }
}

void initializeData()
{
    for (int i = 0; i < teamCount; i++)
    {
        team_data[i].team_id = i + 1;
        strcpy(team_data[i].name, teams[i]);
        team_data[i].total_players = 0;
        team_data[i].avg_batting_strike_rate = 0.0;
        team_data[i].players = NULL;
        team_data[i].batsmen_sorted = NULL;
        team_data[i].bowlers_sorted = NULL;
        team_data[i].all_rounders_sorted = NULL;
    }

    for (int i = 0; i < playerCount; i++)
    {
        const Player *player_info = &players[i];

        int teamIdx = -1;
        for (int j = 0; j < teamCount; j++)
        {
            if (strcmp(player_info->team, teams[j]) == 0)
            {
                teamIdx = j;
                break;
            }
        }

        if (teamIdx == -1)
        {
            continue;
        }

        PlayerNode *new_player = (PlayerNode *)malloc(sizeof(PlayerNode));
        new_player->player_id = player_info->id;
        strcpy(new_player->name, player_info->name);
        strcpy(new_player->team_name, player_info->team);
        new_player->role = getRoleFromString(player_info->role);
        new_player->total_runs = player_info->totalRuns;
        new_player->batting_average = player_info->battingAverage;
        new_player->strike_rate = player_info->strikeRate;
        new_player->wickets = player_info->wickets;
        new_player->economy_rate = player_info->economyRate;
        new_player->performance_index = calculatePerformanceIndex(
            new_player->role, new_player->batting_average, new_player->strike_rate,
            new_player->wickets, new_player->economy_rate);
        new_player->next = NULL;

        PlayerNode *player_copy = (PlayerNode *)malloc(sizeof(PlayerNode));
        *player_copy = *new_player;
        player_copy->next = NULL;

        if (team_data[teamIdx].players == NULL)
        {
            team_data[teamIdx].players = player_copy;
        }
        else
        {
            PlayerNode *temp = team_data[teamIdx].players;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = player_copy;
        }
        team_data[teamIdx].total_players++;

        if (new_player->role == BATSMAN)
        {
            insertPlayerInTeamRoleList(&team_data[teamIdx].batsmen_sorted, new_player);
            insertPlayerSorted(&role_lists.batsmen, new_player);
        }
        else if (new_player->role == BOWLER)
        {
            insertPlayerInTeamRoleList(&team_data[teamIdx].bowlers_sorted, new_player);
            insertPlayerSorted(&role_lists.bowlers, new_player);
        }
        else
        {
            insertPlayerInTeamRoleList(&team_data[teamIdx].all_rounders_sorted, new_player);
            insertPlayerSorted(&role_lists.all_rounders, new_player);
        }
    }

    for (int i = 0; i < teamCount; i++)
    {
        calculateTeamStats(&team_data[i]);
    }
}

void calculateTeamStats(Team *team)
{
    float totalStrikeRate = 0.0;
    int count = 0;

    PlayerNode *players_list = team->players;
    while (players_list != NULL)
    {
        if (players_list->role == BATSMAN || players_list->role == ALL_ROUNDER)
        {
            totalStrikeRate += players_list->strike_rate;
            count++;
        }
        players_list = players_list->next;
    }

    team->avg_batting_strike_rate = count > 0 ? totalStrikeRate / count : 0.0;
}

int binarySearchTeam(int team_id)
{
    int left = 0, right = teamCount - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (team_data[mid].team_id == team_id)
        {
            return mid;
        }
        if (team_data[mid].team_id < team_id)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}

void insertPlayerSorted(RolePlayerNode **head, PlayerNode *player)
{
    RolePlayerNode *newNode = (RolePlayerNode *)malloc(sizeof(RolePlayerNode));
    newNode->player = player;
    newNode->next = NULL;

    if (*head == NULL || (*head)->player->performance_index < player->performance_index)
    {
        newNode->next = *head;
        *head = newNode;
    }
    else
    {
        RolePlayerNode *players_list = *head;
        while (players_list->next != NULL && players_list->next->player->performance_index >= player->performance_index)
        {
            players_list = players_list->next;
        }
        newNode->next = players_list->next;
        players_list->next = newNode;
    }
}

void addPlayerToTeam()
{
    int team_id;
    printf("\nEnter Team ID to add player: ");
    if (scanf("%d", &team_id) != 1)
    {
        printf("\nError. Enter number");
        cleanInputBuffer();
        return;
    }

    int teamIdx = binarySearchTeam(team_id);
    if (teamIdx == -1)
    {
        printf("Invalid Team ID!\n");
        return;
    }

    PlayerNode *new_player = (PlayerNode *)malloc(sizeof(PlayerNode));
    if (new_player == NULL)
    {
        printf("\nMemory allocation failed");
        return;
    }

    printf("Enter Player Details:\n");
    printf("Player ID: ");
    if (scanf("%d", &new_player->player_id) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    PlayerNode *temp = team_data[teamIdx].players;
    while (temp != NULL)
    {
        if (temp->player_id == new_player->player_id)
        {
            printf("Error: Player with ID %d already exists in this team\n", new_player->player_id);
            free(new_player);
            return;
        }
        temp = temp->next;
    }

    cleanInputBuffer();
    printf("Name: ");
    fgets(new_player->name, NAME_LENGTH, stdin);
    new_player->name[strcspn(new_player->name, "\n")] = '\0';

    int role_input;
    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role_input) != 1 || role_input < 1 || role_input > 3)
    {
        printf("\nInvalid role");
        free(new_player);
        return;
    }
    new_player->role = (Role)role_input;

    printf("Total Runs: ");
    if (scanf("%d", &new_player->total_runs) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    printf("Batting Average: ");
    if (scanf("%f", &new_player->batting_average) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    printf("Strike Rate: ");
    if (scanf("%f", &new_player->strike_rate) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    printf("Wickets: ");
    if (scanf("%d", &new_player->wickets) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    printf("Economy Rate: ");
    if (scanf("%f", &new_player->economy_rate) != 1)
    {
        printf("\nError. Enter number");
        free(new_player);
        return;
    }

    strcpy(new_player->team_name, team_data[teamIdx].name);
    new_player->performance_index = calculatePerformanceIndex(
        new_player->role, new_player->batting_average, new_player->strike_rate,
        new_player->wickets, new_player->economy_rate);
    new_player->next = NULL;

    PlayerNode *player_copy = (PlayerNode *)malloc(sizeof(PlayerNode));
    *player_copy = *new_player;
    player_copy->next = NULL;

    if (team_data[teamIdx].players == NULL)
    {
        team_data[teamIdx].players = player_copy;
    }
    else
    {
        PlayerNode *temp = team_data[teamIdx].players;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = player_copy;
    }
    team_data[teamIdx].total_players++;

    if (new_player->role == BATSMAN)
    {
        insertPlayerInTeamRoleList(&team_data[teamIdx].batsmen_sorted, new_player);
        insertPlayerSorted(&role_lists.batsmen, new_player);
    }
    else if (new_player->role == BOWLER)
    {
        insertPlayerInTeamRoleList(&team_data[teamIdx].bowlers_sorted, new_player);
        insertPlayerSorted(&role_lists.bowlers, new_player);
    }
    else
    {
        insertPlayerInTeamRoleList(&team_data[teamIdx].all_rounders_sorted, new_player);
        insertPlayerSorted(&role_lists.all_rounders, new_player);
    }

    calculateTeamStats(&team_data[teamIdx]);

    printf("\nPlayer added successfully to Team %s!\n", team_data[teamIdx].name);
}

void displayTeamPlayers()
{
    int team_id;
    printf("\nEnter Team ID: ");
    if (scanf("%d", &team_id) != 1)
    {
        printf("\nError. Enter number");
        return;
    }

    int teamIdx = binarySearchTeam(team_id);
    if (teamIdx == -1)
    {
        printf("Invalid Team ID!\n");
        return;
    }

    Team *team = &team_data[teamIdx];
    printf("\nPlayers of Team %s:\n", team->name);
    printf("====================================================================================\n");
    printf("%-6s %-25s %-15s %-6s %-6s %-6s %-6s %-6s %-10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================\n");

    PlayerNode *players_list = team->players;
    while (players_list != NULL)
    {
        printf("%-6d %-25s %-15s %-6d %-6.1f %-6.1f %-6d %-6.1f %-10.2f\n",
               players_list->player_id, players_list->name, getRoleString(players_list->role),
               players_list->total_runs, players_list->batting_average, players_list->strike_rate,
               players_list->wickets, players_list->economy_rate, players_list->performance_index);
        players_list = players_list->next;
    }

    printf("====================================================================================\n");
    printf("Total Players: %d\n", team->total_players);
    printf("Average Batting Strike Rate: %.2f\n", team->avg_batting_strike_rate);
}

void displayTeamsByStrikeRate()
{
    Team *sortedTeams[MAX_TEAMS];
    for (int i = 0; i < teamCount; i++)
    {
        sortedTeams[i] = &team_data[i];
    }

    for (int i = 0; i < teamCount - 1; i++)
    {
        for (int j = 0; j < teamCount - i - 1; j++)
        {
            if (sortedTeams[j]->avg_batting_strike_rate < sortedTeams[j + 1]->avg_batting_strike_rate)
            {
                Team *temp = sortedTeams[j];
                sortedTeams[j] = sortedTeams[j + 1];
                sortedTeams[j + 1] = temp;
            }
        }
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("=========================================================\n");
    printf("%-4s %-20s %-12s %-15s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("=========================================================\n");

    for (int i = 0; i < teamCount; i++)
    {
        printf("%-4d %-20s %-12.1f %-15d\n",
               sortedTeams[i]->team_id, sortedTeams[i]->name,
               sortedTeams[i]->avg_batting_strike_rate, sortedTeams[i]->total_players);
    }
    printf("=========================================================\n");
}

void displayTopKPlayers()
{
    int team_id, role_input, k;
    printf("\nEnter Team ID: ");
    if (scanf("%d", &team_id) != 1)
    {
        printf("\nError. Enter number");
        return;
    }

    int teamIdx = binarySearchTeam(team_id);
    if (teamIdx == -1)
    {
        printf("Invalid Team ID!\n");
        return;
    }

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role_input) != 1 || role_input < 1 || role_input > 3)
    {
        printf("\nInvalid role");
        return;
    }
    Role role = (Role)role_input;

    printf("Enter number of players: ");
    if (scanf("%d", &k) != 1 || k <= 0)
    {
        printf("\nError. Enter positive number");
        return;
    }

    PlayerNode *sortedList = NULL;
    if (role == BATSMAN)
    {
        sortedList = team_data[teamIdx].batsmen_sorted;
    }
    else if (role == BOWLER)
    {
        sortedList = team_data[teamIdx].bowlers_sorted;
    }
    else
    {
        sortedList = team_data[teamIdx].all_rounders_sorted;
    }

    printf("\nTop %d %ss of Team %s:\n", k, getRoleString(role), team_data[teamIdx].name);
    printf("====================================================================================\n");
    printf("%-6s %-25s %-15s %-6s %-6s %-6s %-6s %-6s %-10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("====================================================================================\n");

    PlayerNode *players_list = sortedList;
    int count = 0;
    while (players_list != NULL && count < k)
    {
        printf("%-6d %-25s %-15s %-6d %-6.1f %-6.1f %-6d %-6.1f %-10.2f\n",
               players_list->player_id, players_list->name, getRoleString(players_list->role),
               players_list->total_runs, players_list->batting_average, players_list->strike_rate,
               players_list->wickets, players_list->economy_rate, players_list->performance_index);
        players_list = players_list->next;
        count++;
    }
    printf("====================================================================================\n");
}

void displayPlayersByRole()
{
    int role_input;
    printf("\nEnter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role_input) != 1 || role_input < 1 || role_input > 3)
    {
        printf("\nInvalid role");
        return;
    }
    Role role = (Role)role_input;

    RolePlayerNode *list = NULL;
    if (role == BATSMAN)
    {
        list = role_lists.batsmen;
    }
    else if (role == BOWLER)
    {
        list = role_lists.bowlers;
    }
    else
    {
        list = role_lists.all_rounders;
    }

    printf("\n%ss of all teams:\n", getRoleString(role));
    printf("======================================================================================\n");
    printf("%-6s %-25s %-15s %-15s %-6s %-6s %-6s %-6s %-6s %-10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("======================================================================================\n");

    RolePlayerNode *players_list = list;
    while (players_list != NULL)
    {
        PlayerNode *player_info = players_list->player;
        printf("%-6d %-25s %-15s %-15s %-6d %-6.1f %-6.1f %-6d %-6.1f %-10.2f\n",
               player_info->player_id, player_info->name, player_info->team_name, getRoleString(player_info->role),
               player_info->total_runs, player_info->batting_average, player_info->strike_rate,
               player_info->wickets, player_info->economy_rate, player_info->performance_index);
        players_list = players_list->next;
    }
    printf("======================================================================================\n");
}

void displayMenu()
{
    printf("\n==============================================================================\n");
    printf("                  ICC ODI Player Performance Analyzer\n");
    printf("==============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf("==============================================================================\n");
    printf("Enter your choice: ");
}

void freeMemory()
{
    for (int i = 0; i < teamCount; i++)
    {
        PlayerNode *players_list = team_data[i].players;
        while (players_list != NULL)
        {
            PlayerNode *temp = players_list;
            players_list = players_list->next;
            free(temp);
        }
    }

    RolePlayerNode *currRole = role_lists.batsmen;
    while (currRole != NULL)
    {
        RolePlayerNode *temp = currRole;
        currRole = currRole->next;
        free(temp);
    }

    currRole = role_lists.bowlers;
    while (currRole != NULL)
    {
        RolePlayerNode *temp = currRole;
        currRole = currRole->next;
        free(temp);
    }

    currRole = role_lists.all_rounders;
    while (currRole != NULL)
    {
        RolePlayerNode *temp = currRole;
        currRole = currRole->next;
        free(temp);
    }
}

void cleanInputBuffer()
{
    int buffer;
    while ((buffer = getchar()) != '\n')
        ;
}